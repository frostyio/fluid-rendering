#include "core/renderer.hpp"
#include <iostream>

using namespace engine;

Renderer::Renderer(int width, int height)
	: dummy2DTexture(0), dummyCubemapTexture(0),
	  dummyTexturesInitialized(false), width(width), height(height) {
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
	InitializeDummyTextures();

	// default framebuffers
	CreateBuffer("opaque", width, height, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				 GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, true);
	CreateBuffer("trans", width, height);
	CreateBuffer("post", width, height);

	// quad
	float quadVertices[] = {
		-1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
		0.0f,  0.0f, 1.0f, -1.0f, 1.0f,	 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,  1.0f,	 -1.0f,
		1.0f,  0.0f, 1.0f, 1.0f,  1.0f,	 1.0f,
	};

	glGenVertexArrays(1, &fullscreenQuadVAO);
	glBindVertexArray(fullscreenQuadVAO);

	glGenBuffers(1, &fullscreenQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
				 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
						  (void *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
						  (void *)(2 * sizeof(float)));

	glBindVertexArray(0);
}

Renderer::~Renderer() {
	if (dummy2DTexture != 0) {
		glDeleteTextures(1, &dummy2DTexture);
	}

	if (dummyCubemapTexture != 0) {
		glDeleteTextures(1, &dummyCubemapTexture);
	}
}

void Renderer::CreateProgram(std::string name, GLSLProgram *prog) {
	if (programs.count(name)) {
		std::cout << "'" << name << "' already exists as a program"
				  << std::endl;
		return;
	}

	programs[name] = prog;
	std::cout << "created: " << name << " [" << prog->GetID() << "]"
			  << std::endl;
}

GLSLProgram *Renderer::GetProgram(std::string name) {
	return programs.at(name);
}

void Renderer::BindProgram(std::string name) {
	GLSLProgram *prog = GetProgram(name);
	currentlyBinded = name;
	prog->Bind();
	if (samplersByProgram.find(prog->GetID()) == samplersByProgram.end()) {
		CollectSamplerUniforms(prog->GetID());
	}
	// std::cout << "binded: " << name << " [" << prog->GetID() << "]"
	// 		  << std::endl;
}

void Renderer::BeginFrame() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame(GLFWwindow *window) { glfwSwapBuffers(window); }

void Renderer::DrawMesh() {
	SetUniform("shading", static_cast<int>(shadingType));
}

void Renderer::BindTexture(const char *name, GLuint textureID,
						   GLenum textureUnit, GLenum type) {
	glActiveTexture(textureUnit);
	glBindTexture(type, textureID);
	SetUniform(name, (int)(textureUnit - GL_TEXTURE0));
}

void Renderer::InitializeDummyTextures() {
	if (dummyTexturesInitialized) {
		return;
	}

	dummy2DTexture = CreateDummyTexture2D();
	dummyCubemapTexture = CreateDummyCubemap();

	dummyTexturesInitialized = true;
}

GLuint Renderer::CreateDummyTexture2D() {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	unsigned char data[] = {0, 0, 0, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				 data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureId;
}

GLuint Renderer::CreateDummyCubemap() {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	unsigned char data[] = {0, 0, 0, 255};

	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 1, 1, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureId;
}

void Renderer::CollectSamplerUniforms(GLuint programId) {
	std::vector<SamplerInfo> samplers;

	GLint numUniforms = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);

	GLint maxNameLength = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

	std::vector<char> nameBuffer(maxNameLength);

	GLint textureUnit = 0;

	for (GLint i = 0; i < numUniforms; i++) {
		GLint size;
		GLenum type;
		GLsizei actualLength;

		glGetActiveUniform(programId, i, maxNameLength, &actualLength, &size,
						   &type, nameBuffer.data());

		if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE) {
			SamplerInfo info;
			info.name = std::string(nameBuffer.data(), actualLength);
			info.type = type;
			info.location = glGetUniformLocation(programId, info.name.c_str());
			info.textureUnit = textureUnit++;

			samplers.push_back(info);
		}
	}

	samplersByProgram[programId] = samplers;
}

void Renderer::SetDummyTextures() {
	if (!dummyTexturesInitialized) {
		InitializeDummyTextures();
	}

	if (currentlyBinded == "") {
		return;
	}

	auto it = samplersByProgram.find(GetProgram(currentlyBinded)->GetID());
	if (it == samplersByProgram.end()) {
		return;
	}

	for (const auto &sampler : it->second) {
		glActiveTexture(GL_TEXTURE0 + sampler.textureUnit);

		if (sampler.type == GL_SAMPLER_2D) {
			glBindTexture(GL_TEXTURE_2D, dummy2DTexture);
		} else if (sampler.type == GL_SAMPLER_CUBE) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, dummyCubemapTexture);
		}

		glUniform1i(sampler.location, sampler.textureUnit);
	}
}

void Renderer::Composite() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	this->BindProgram("_composite");

	this->BindTexture("uScene", framebuffers["opaque"].texture, GL_TEXTURE0);
	this->BindTexture("uTrans", framebuffers["trans"].texture, GL_TEXTURE1);
	this->BindTexture("uPost", framebuffers["post"].texture, GL_TEXTURE2);

	DrawFullscreenQuad();

	glDisable(GL_BLEND);
}
