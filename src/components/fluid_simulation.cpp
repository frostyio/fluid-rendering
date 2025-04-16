#include "components/fluid_simulation.hpp"
#include "common/typedefs.hpp"
#include "core/renderer.hpp"
#include "core/scene_object.hpp"
#include "objects/camera.hpp"
#include "objects/skybox.hpp"
#include <optional>

using namespace engine;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCoreFactory;

void findPointsRecursive(const Alembic::Abc::IObject &obj) {
	const auto &header = obj.getHeader();
	std::string schema = header.getMetaData().get("schema");

	std::cout << "checking: " << obj.getFullName() << " (" << schema << ")\n";

	if (IPoints::matches(header)) {
		IPoints points(obj, Alembic::Abc::kWrapExisting);
		IPointsSchema::Sample sample;
		points.getSchema().get(sample);
		P3fArraySamplePtr positions = sample.getPositions();

		if (positions) {
			std::cout << "  >> found points: " << obj.getFullName() << "\n";
			std::cout << "  >> num points: " << positions->size() << "\n";
		} else {
			std::cout << "  >> points object has no position data\n";
		}
	}

	for (size_t i = 0; i < obj.getNumChildren(); ++i) {
		findPointsRecursive(obj.getChild(i));
	}
}

void extractPointsFrames(const Alembic::AbcGeom::IPoints &points,
						 std::vector<std::vector<Vec3f>> &allFrames) {
	const auto &schema = points.getSchema();
	size_t numSamples = schema.getNumSamples();

	for (size_t i = 0; i < numSamples; ++i) {
		Alembic::AbcGeom::IPointsSchema::Sample sample;
		schema.get(sample, i);

		auto positions = sample.getPositions();
		std::vector<Vec3f> frameVertices;

		if (positions) {
			for (size_t j = 0; j < positions->size(); ++j) {
				auto p = (*positions)[j];
				frameVertices.emplace_back(p.x, p.y, p.z);
			}
		}

		allFrames.push_back(std::move(frameVertices));
	}
	std::cout << "  >> got " << allFrames.size() << " frames from "
			  << points.getFullName() << std::endl;
}

void findAndExtractPointsRecursive(const Alembic::Abc::IObject &obj,
								   std::vector<std::vector<Vec3f>> &allFrames) {

	const auto &header = obj.getHeader();
	if (IPoints::matches(header)) {
		IPoints points(obj, Alembic::Abc::kWrapExisting);
		extractPointsFrames(points, allFrames);
		return;
	}

	for (size_t i = 0; i < obj.getNumChildren(); ++i) {
		findAndExtractPointsRecursive(obj.getChild(i), allFrames);
	}
}

std::optional<BakedPointDataComponent>
BakedPointDataComponent::create(const std::string &path) {
	IFactory factory;
	IFactory::CoreType coreType;
	IArchive archive = factory.getArchive(path, coreType);

	if (!archive.valid()) {
		std::cerr << "failed to open Alembic file: " << path << std::endl;
		return std::nullopt;
	}

	std::cout << "opened Alembic file: " << path << std::endl;

	return BakedPointDataComponent(archive);
}

BakedPointDataComponent::BakedPointDataComponent(const IArchive &archive) {
	std::vector<std::vector<Vec3f>> allFrames;
	findAndExtractPointsRecursive(archive.getTop(), allFrames);

	int maxPoints = allFrames[0].size();
	for (const auto &frame : allFrames) {
		if ((int)frame.size() > maxPoints)
			maxPoints = frame.size();
	}

	std::cout << "total frames read: " << allFrames.size() << std::endl;
	std::cout << "max points: " << (allFrames.empty() ? 0 : maxPoints)
			  << std::endl;

	std::vector<Vec3f> allFrameData;
	currentFrame = 0;

	numPoints = maxPoints;
	numFrames = allFrames.size();

	for (auto &frame : allFrames) {
		frame.resize(maxPoints, Vec3f(0.0f, 1000.0f, 0.0f)); // pad
		allFrameData.insert(allFrameData.end(), frame.begin(), frame.end());
	}

	std::cout << "made frame data!" << std::endl;

	// frame data
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glBufferData(GL_ARRAY_BUFFER, allFrameData.size() * sizeof(Vec3f),
				 allFrameData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// depth mapping
	// a
	glGenFramebuffers(1, &depthFBOA);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBOA);

	glGenTextures(1, &depthTextureA);
	glBindTexture(GL_TEXTURE_2D, depthTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, frameWidth,
				 frameHeight, // width and height hard-coded for now
				 0, GL_RED, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   depthTextureA, 0);
	GLuint depthRenderbuffer;
	glGenRenderbuffers(1, &depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, frameWidth,
						  frameHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, depthRenderbuffer);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	// b
	glGenFramebuffers(1, &depthFBOB);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBOB);

	glGenTextures(1, &depthTextureB);
	glBindTexture(GL_TEXTURE_2D, depthTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, frameWidth,
				 frameHeight, // width and height hard-coded for now
				 0, GL_RED, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   depthTextureB, 0);
	GLuint depthRenderbuffer2;
	glGenRenderbuffers(1, &depthRenderbuffer2);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, frameWidth,
						  frameHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, depthRenderbuffer2);

	glDrawBuffers(1, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// filtering
	glGenFramebuffers(1, &filterFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, filterFBO);

	glGenTextures(1, &filteredDepthTexture);
	glBindTexture(GL_TEXTURE_2D, filteredDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, frameWidth,
				 frameHeight, // more hard-coded
				 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   filteredDepthTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// normal frame buffer
	glGenFramebuffers(1, &normalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);

	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, frameWidth, frameHeight, 0,
				 GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// idk if this does what i want
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   normalTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// thickness

	glGenTextures(1, &thicknessTexture);
	glBindTexture(GL_TEXTURE_2D, thicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, frameWidth, frameHeight, 0, GL_RED,
				 GL_FLOAT, // hard-coded
				 nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &thicknessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, thicknessFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   thicknessTexture, 0);
}

void BakedPointDataComponent::Bind() { glBindVertexArray(vao); }

void BakedPointDataComponent::Update(double dt) {
	timer += dt;
	if (timer >= (1. / 60.)) {
		currentFrame++;
		if (currentFrame >= numFrames) {
			currentFrame = 0;
			loopCount++;
		}
		timer = 0;
	}
}

void BakedPointDataComponent::Draw(Renderer &renderer, Scene *scene,
								   Matrix4f model) {
	SceneObject *owner = GetOwner();
	if (owner != nullptr) {
		GLuint old = CurrentDrawFBO();

		constexpr int pointSize = 10;

		// thickness
		renderer.BindProgram("thicknessMap");
		glBindFramebuffer(GL_FRAMEBUFFER, thicknessFBO);
		glViewport(0, 0, frameWidth, frameHeight); // hard-coded
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		Bind();
		renderer.SetModel(model);
		renderer.SetView(scene->GetActiveCamera()->GetView());
		renderer.SetProjection(scene->GetActiveCamera()->GetProjection());
		renderer.SetUniform("pointSize", pointSize * 2);
		glDrawArrays(GL_POINTS, currentFrame * numPoints, numPoints);
		glDisable(GL_BLEND);

		// PARTICLE DEPTH MAP
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS); // or GL_LEQUAL
		glDisable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFBOA);
		glViewport(0, 0, frameWidth, frameHeight); // hard-coded
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.BindProgram("waterDepth");
		renderer.SetModel(model);
		renderer.SetView(scene->GetActiveCamera()->GetView());
		renderer.SetProjection(scene->GetActiveCamera()->GetProjection());
		renderer.SetUniform("pointSize", pointSize);
		Bind(); // binds vao
		glDrawArrays(GL_POINTS, currentFrame * numPoints, numPoints);

		// NARROW FILTER

		int numPasses = 3;
		bool ping = true;
		GLuint inputTex = depthTextureA;
		GLuint outputTex = depthTextureB;

		float aspect = (float)frameWidth / (float)frameHeight;
		float fov_v_rad =
			2.0f *
			std::atan(
				std::tan(deg2rad(scene->GetActiveCamera()->GetFov()) / 2.0f) /
				aspect);
		float r = pointSize;

		for (int i = 0; i < numPasses; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, filterFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D, outputTex, 0);
			glViewport(0, 0, frameWidth, frameHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderer.BindProgram("narrowFilter");
			renderer.BindTexture("uDepthTex", inputTex, GL_TEXTURE0);

			renderer.SetUniform("uDelta", 10 * r);
			renderer.SetUniform("uMu", r);
			renderer.SetUniform("uWorldSigma", 0.7f * r);

			renderer.SetUniform("uFOV", fov_v_rad);
			renderer.SetUniform("uScreenHeight", (float)frameHeight);

			renderer.DrawFullscreenQuad();

			std::swap(inputTex, outputTex);
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, ping ? depthFBOB : depthFBOA);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							   GL_TEXTURE_2D, filteredDepthTexture, 0);
		glBlitFramebuffer(0, 0, frameWidth, frameHeight, 0, 0, frameWidth,
						  frameHeight,
						  GL_COLOR_BUFFER_BIT, // hard-coded
						  GL_NEAREST);

		// NORMAL RECONSTRUCTION
		glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
		glViewport(0, 0, frameWidth, frameHeight); // hard-coded
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.BindProgram("normalReconstruction");
		renderer.BindTexture("uFilteredDepth", filteredDepthTexture,
							 GL_TEXTURE0);
		renderer.SetUniform("uFOV", fov_v_rad);
		renderer.SetUniform("uScreenHeight", (float)frameHeight); // hard-coded

		renderer.DrawFullscreenQuad();

		// RENDERING

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		renderer.BindProgram("fluidProgram");

		renderer.SetModel(model);
		renderer.SetView(scene->GetActiveCamera()->GetView());
		renderer.SetProjection(scene->GetActiveCamera()->GetProjection());
		renderer.SetUniform("viewPos", scene->GetActiveCamera()->GetPosition());
		renderer.SetUniform("lightPos", scene->GetSunPosition());

		renderer.SetUniform("materialType", 1);
		renderer.SetUniform("shading", 1);

		renderer.SetUniform("shininess", 64.0f);
		renderer.SetUniform("ambientColor", Vec3f(0.1f, 0.2f, 0.25f));
		renderer.SetUniform("diffuseColor", Vec3f(0.25f, 0.55f, 0.75f));
		renderer.SetUniform("specularColor", Vec3f(1.0f, 1.0f, 1.0f));

		renderer.BindTexture("uNormalTex", normalTexture, GL_TEXTURE0);
		renderer.BindTexture("uDepthTex", filteredDepthTexture, GL_TEXTURE1);
		renderer.BindTexture("uThicknessTex", thicknessTexture, GL_TEXTURE2);
		glActiveTexture(GL_TEXTURE3);
		scene->GetActiveSkybox()->GetTexture().Bind();
		renderer.SetUniform("uSkyboxTex", 3);
		renderer.BindTexture("uOpaqueDepthTex",
							 renderer.FindBuffer("opaque")->depthTex,
							 GL_TEXTURE4);
		renderer.BindTexture("uBackgroundColorTex",
							 renderer.FindBuffer("opaque")->texture,
							 GL_TEXTURE5);

		renderer.SetUniform("uFovY", fov_v_rad);
		renderer.SetUniform("uAspect", aspect);
		// renderer.SetUniform("uTime", (float)glfwGetTime());
		renderer.SetUniform("uTime", fmod((float)glfwGetTime(), 60.0f));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		renderer.BindBuffer(old);
		renderer.DrawFullscreenQuad();

		// DEBUG
		// glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// renderer.BindBuffer(old);
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// renderer.BindProgram("debugDisplay");
		// renderer.BindTexture("uTexture", depthTextureA);
		// renderer.BindTexture("uTexture", normalTexture);
		// renderer.BindTexture("uTexture",
		// renderer.FindBuffer("opaque")->texture);
		// renderer.DrawFullscreenQuad();

		// revert if we changed & then render normally
		// glDisable(GL_BLEND);
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// glViewport(0, 0, 640, 480); // HARD-CODED, BAD

		// renderer.BindProgram("default");
		// renderer.SetUniform("materialType", 0);

		// glClear(GL_DEPTH_BUFFER_BIT);
		// Bind();
		// glDrawArrays(GL_POINTS, currentFrame * numPoints, numPoints);
	} else {
		// render normally
		glDrawArrays(GL_POINTS, currentFrame * numPoints, numPoints);
	}
}

bool BakedPointDataComponent::IsFinished() { return loopCount > 0; }
void BakedPointDataComponent::Reset() {
	timer = 0;
	loopCount = 0;
	currentFrame = 0;
}

void FluidSimulationComponent::Bind() {}
void FluidSimulationComponent::Update(double) {}
void FluidSimulationComponent::Draw(Renderer &renderer, Scene *scene,
									Matrix4f model) {}
bool FluidSimulationComponent::IsFinished() { return true; }
void FluidSimulationComponent::Reset() {}
