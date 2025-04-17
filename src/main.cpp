#include "common/typedefs.hpp"
#include "components/fluid_simulation.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"
#include "core/scene_object.hpp"
#include "objects/camera.hpp"
#include "objects/fluid.hpp"
#include "objects/mesh.hpp"
#include "objects/skybox.hpp"
#include <future>
#include <memory>
#include <unordered_map>
#include <vector>
#undef min
#undef max

static cy::Vec2f windowSize;

static cy::Vec2<double> mouseChange(0.0, 0.0);
static cy::Vec2<double> lastMousePos(0.0, 0.0);
static bool isMouse1Pressed = false;
static bool isMouse2Pressed = false;
static cy::Vec2<double> accumulatedDrag(30., 20.);
static double accumulatedZoom = 70.;

static int sceneIndex = 0;
static engine::Scene *currentScene = nullptr;

static bool paused = false;

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
						int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS && currentScene != nullptr) {
		SceneObject *fluidObject = currentScene->GetObject("fluid");
		FluidObject *fluid = nullptr;
		if (fluidObject != nullptr)
			fluid = dynamic_cast<FluidObject *>(fluidObject);

		if (fluid == nullptr)
			return;

		if (key == GLFW_KEY_R)
			fluid->Reset();
		else if (key == GLFW_KEY_D)
			sceneIndex++;
		else if (key == GLFW_KEY_A)
			sceneIndex--;
		else if (key == GLFW_KEY_SPACE)
			paused = !paused;
	}
}

static void mouseCallback(GLFWwindow *window, int button, int action,
						  int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2) {
		if (action == GLFW_PRESS) {
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);
			lastMousePos = cy::Vec2(xPos, yPos);
			if (button == GLFW_MOUSE_BUTTON_1)
				isMouse1Pressed = true;
			else
				isMouse2Pressed = true;
		} else if (action == GLFW_RELEASE) {
			mouseChange = cy::Vec2(0.0, 0.0);
			if (button == GLFW_MOUSE_BUTTON_1)
				isMouse1Pressed = false;
			else
				isMouse2Pressed = false;
		}
	}
}

cy::Vec2<double> getMouseDelta(GLFWwindow *window) {
	if (isMouse1Pressed || isMouse2Pressed) {
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		cy::Vec2<double> currentMousePos(xPos, yPos);

		mouseChange = currentMousePos - lastMousePos;
		lastMousePos = currentMousePos;
		return mouseChange;
	}

	return cy::Vec2(0.0, 0.0);
}

Quatf FromAxisAngle(const Vec3f &axis, float angle_rad) {
	float half_angle = angle_rad * 0.5f;
	float s = sinf(half_angle);
	return {cosf(half_angle), axis.x * s, axis.y * s, axis.z * s};
}

engine::Scene *makeDefaultScene() {
	engine::Scene *scene = new engine::Scene();
	scene->SetSunPosition({30, 20, 30});

	engine::CameraObject *camera = new engine::CameraObject({0, 0, 0});
	scene->AddObject(std::unique_ptr<engine::SceneObject>(camera));
	scene->SetActiveCamera(camera);
	camera->SetAspectRatio(windowSize.x / windowSize.y);

	{
		const std::vector<Vertex> PLANE_VERTICES = {
			{{-1.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}},
			{{1.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}},
			{{1.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}},
			{{-1.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 0.0}}};
		const std::vector<unsigned int> PLANE_INDICES = {0, 1, 2, 2, 3, 0};
		engine::MeshObject *meshObject =
			new engine::MeshObject(PLANE_VERTICES, PLANE_INDICES);
		meshObject->SetPosition({0, -22, 0});
		meshObject->SetMeshSize({200, 1, 200});
		meshObject->SetMeshColor({.2, .2, .2});
		scene->AddObject(std::unique_ptr<engine::SceneObject>(meshObject));
		// meshObject->SetTextures("assets/textures/stone_tile");
		meshObject->SetTextures("assets/textures/brick");
	}

	engine::SkyboxObject *object = new engine::SkyboxObject();
	scene->AddObject(std::unique_ptr<engine::SceneObject>(object));
	scene->SetActiveSkybox(object);

	return scene;
}

auto ObjectMakerFor(engine::Scene *scene) {
	return [=](const Vec3f &position, const Vec3f &size) {
		const float scale = 20.5f;
		engine::MeshObject *meshObject = new engine::MeshObject(size * scale);
		meshObject->SetPosition(position * scale);
		meshObject->SetMeshColor({0.7f, 0.2f, 0.2f});
		meshObject->SetTextures("assets/textures/brick");
		scene->AddObject(std::unique_ptr<engine::SceneObject>(meshObject));
	};
}

using CacheMap =
	std::unordered_map<std::string,
					   std::tuple<std::vector<Vec3f>, size_t, size_t>>;

engine::Scene *sceneOne(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	const auto &[frameData, nPoints, nFrames] = cache.at("scene1");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	return scene;
}

engine::Scene *sceneTwo(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene2");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	MakeObject({1, -0.5, 1}, {.97, .97, .97});

	return scene;
}

engine::Scene *sceneThree(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene3");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	MakeObject({1, -0.5, 1}, {.97, .97, .97});

	return scene;
}

engine::Scene *sceneFour(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene4");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	return scene;
}

engine::Scene *sceneFive(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene5");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	return scene;
}

engine::Scene *sceneSix(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene6");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	return scene;
}

engine::Scene *sceneSeven(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene7");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	return scene;
}

engine::Scene *sceneEight(const CacheMap &cache) {
	engine::Scene *scene = makeDefaultScene();
	auto MakeObject = ObjectMakerFor(scene);
	const auto &[frameData, nPoints, nFrames] = cache.at("scene8");

	engine::FluidObject *object = new engine::FluidObject();
	object->fromFrameData(frameData, nPoints, nFrames);
	object->SetPosition({0, -30, 0});
	object->SetSize({20, 20, 20});
	scene->AddObject("fluid", std::unique_ptr<engine::SceneObject>(object));

	MakeObject({1, -0.5, 0}, {.97, .97, .97});

	return scene;
}

std::vector<engine::Scene *> makeScenes() {

	const std::unordered_map<std::string, std::string> usedCaches = {
		{"scene3", "assets/caches/prodScene2-3.abc"},
		{"scene5", "assets/caches/prodScene3-1.abc"},
		{"scene8", "assets/caches/prodScene4-3.abc"},
#ifndef MINIMAL

		{"scene1", "assets/caches/prodScene1.abc"},
		{"scene2", "assets/caches/prodScene2-2.abc"},
		{"scene4", "assets/caches/prodScene3.abc"},
		{"scene6", "assets/caches/prodScene3-2.abc"},
		{"scene7", "assets/caches/prodScene4-1.abc"},
#endif

	};

	const std::vector<std::function<engine::Scene *(const CacheMap &)>>
		sceneGenerators = {
			sceneThree, sceneFive, sceneEight,
#ifndef MINIMAL
			sceneOne,	sceneTwo,  sceneFour,  sceneSix, sceneSeven,
#endif

		};

	CacheMap loadedCaches;

	std::vector<std::future<
		std::pair<std::string, std::tuple<std::vector<Vec3f>, size_t, size_t>>>>
		futures;

	for (const auto &entry : usedCaches) {
		const std::string name = entry.first;
		const std::string path = entry.second;

		futures.emplace_back(std::async(
			std::launch::async,
			[name, path]()
				-> std::pair<std::string,
							 std::tuple<std::vector<Vec3f>, size_t, size_t>> {
				size_t nPoints = 0, nFrames = 0;
				auto frameData =
					BakedPointDataComponent::createFrameDataFromPath(
						path, nPoints, nFrames);
				if (!frameData)
					throw std::runtime_error("failed to load " + path);
				return {name, {std::move(*frameData), nPoints, nFrames}};
			}));
	}

	for (auto &f : futures) {
		auto [name, data] = f.get();
		loadedCaches.emplace(std::move(name), std::move(data));
	}

	std::cout << "loaded all!" << std::endl;

	std::vector<engine::Scene *> scenes;
	for (auto &fn : sceneGenerators) {
		engine::Scene *scene = fn(loadedCaches);
		scenes.push_back(scene);
	}
	return scenes;
}

int main(int argc, char **argv) {
	GLFW_SETUP;

#ifdef PROJECT_NAME
	// CREATE_GLFW_WINDOW(640, 480, PROJECT_NAME);
	CREATE_GLFW_WINDOW(1280, 960, PROJECT_NAME);
#else
	exit(-1);
#endif

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// AFTER OpenGL context is created!
	INIT_GLEW(window);

	// callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
#if !defined(__APPLE__)
	CY_GL_REGISTER_DEBUG_CALLBACK;
	SETUP_DEBUG_CALLBACKS;
#endif

	// opengl
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	windowSize = {(float)width, (float)height};

	//
	engine::Renderer renderer = engine::Renderer(&windowSize);

	// renderer setup
	GLSLProgram prog;
	prog.BuildFiles("assets/shaders/shader.vert",
					"assets/shaders/shading.frag");
	renderer.CreateProgram("default", &prog);
	renderer.GetProgram("default");

	// composite shader
	GLSLProgram compositeProg;
	compositeProg.BuildFiles("assets/shaders/quad.vert",
							 "assets/shaders/composite.frag");
	renderer.CreateProgram("_composite", &compositeProg);

	// WATER
	GLSLProgram waterDepthProgram;
	waterDepthProgram.BuildFiles("assets/shaders/depth_pass.vert",
								 "assets/shaders/depth_pass.frag");
	renderer.CreateProgram("waterDepth", &waterDepthProgram);
	GLSLProgram narrowFilterProgram;
	narrowFilterProgram.BuildFiles("assets/shaders/quad.vert",
								   "assets/shaders/narrow_filter.frag");
	renderer.CreateProgram("narrowFilter", &narrowFilterProgram);
	GLSLProgram debugDisplayProgram;
	debugDisplayProgram.BuildFiles("assets/shaders/quad.vert",
								   "assets/shaders/debug_display.frag");
	renderer.CreateProgram("debugDisplay", &debugDisplayProgram);
	GLSLProgram normalReconstructionProgram;
	normalReconstructionProgram.BuildFiles(
		"assets/shaders/quad.vert",
		"assets/shaders/normal_reconstruction.frag");
	renderer.CreateProgram("normalReconstruction",
						   &normalReconstructionProgram);
	GLSLProgram fluidProgram;
	fluidProgram.BuildFiles("assets/shaders/quad.vert",
							"assets/shaders/shading.frag");
	renderer.CreateProgram("fluidProgram", &fluidProgram);
	GLSLProgram thicknessProgram;
	thicknessProgram.BuildFiles("assets/shaders/depth_pass.vert",
								"assets/shaders/thickness.frag");
	renderer.CreateProgram("thicknessMap", &thicknessProgram);

	// scenes

	auto programScenes = makeScenes();
	currentScene = programScenes.at(sceneIndex);

	glEnable(GL_PROGRAM_POINT_SIZE);

	double lastTimeFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		double currentTimeFrame = glfwGetTime();
		double dt = currentTimeFrame - lastTimeFrame;
		lastTimeFrame = currentTimeFrame;

		renderer.BeginFrame();

		if (isMouse1Pressed)
			accumulatedDrag += getMouseDelta(window) * 0.3;
		if (isMouse2Pressed)
			accumulatedZoom += getMouseDelta(window).y * 0.2;
		float theta = deg2rad(accumulatedDrag.x);
		float phi = deg2rad(accumulatedDrag.y);
		float radius = accumulatedZoom + 30;
		cy::Vec3f cameraPos =
			cy::Vec3f{radius * cos(phi) * sin(theta), radius * sin(phi),
					  radius * cos(phi) * cos(theta)};

		// scene handling
		SceneObject *fluidObject = currentScene->GetObject("fluid");
		FluidObject *fluid = nullptr;
		if (fluidObject != nullptr)
			fluid = dynamic_cast<FluidObject *>(fluidObject);

		if (fluid != nullptr && fluid->IsFinished())
			sceneIndex++;

		if (sceneIndex > programScenes.size() - 1)
			sceneIndex = 0;
		// if (sceneIndex < 0)
		// 	sceneIndex = programScenes.size() - 1;

		if (programScenes.at(sceneIndex) != currentScene) {
			if (fluid != nullptr)
				fluid->Reset();

			currentScene = programScenes.at(sceneIndex);
			std::cout << "moving to scene #" << sceneIndex << std::endl;
		}

		//

		currentScene->GetActiveCamera()->SetPosition(cameraPos);
		if (!paused)
			currentScene->Update(dt);
		currentScene->Render(renderer);

		renderer.EndFrame(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
