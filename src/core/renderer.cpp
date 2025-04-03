#include "core/renderer.hpp"
#include <iostream>

using namespace engine;

Renderer::Renderer(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
}

void Renderer::CreateProgram(std::string name, GLSLProgram *prog) {
	if (programs.count(name)) {
		std::cout << "'" << name << "' already exists as a program"
				  << std::endl;
		return;
	}

	programs[name] = std::ref(prog);
}

GLSLProgram *Renderer::GetProgram(std::string name) {
	return programs.at(name);
}

void Renderer::BindProgram(std::string name) {
	GLSLProgram *prog = GetProgram(name);
	currentlyBinded = name;
	prog->Bind();
}

void Renderer::BeginFrame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame(GLFWwindow *window) { glfwSwapBuffers(window); }

void Renderer::DrawMesh() {
	SetUniform("shading", static_cast<int>(shadingType));
}
