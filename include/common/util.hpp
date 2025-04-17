#ifndef _UTIL_H_
#define _UTIL_H_

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include "cyMatrix.h"
#include "cyTriMesh.h"
#include "cyVector.h"

#include "cyGL.h"

#include <iostream>

// compiler ignore unused
inline void Ignore() {
	(void)sizeof(cyMatrix4f);
	(void)sizeof(cyTriMesh);
	(void)sizeof(cyVec3f);
}

inline float deg2rad(float degrees) {
	static const float pi_180 = 4.0 * atan(1.0) / 180.0;
	return degrees * pi_180;
}

inline float rad2deg(float radians) {
	static const float inv_pi_180 = 180.0 / (4.0 * atan(1.0));
	return radians * inv_pi_180;
}

// "modern" opengl
#define GLFW_SETUP                                                             \
	if (!glfwInit())                                                           \
		exit(-1);                                                              \
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                             \
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                             \
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);             \
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);                              \
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

#define CREATE_GLFW_WINDOW(width, height, title)                               \
	GLFWwindow *window =                                                       \
		glfwCreateWindow(width, height, title, nullptr, nullptr);              \
	if (!window) {                                                             \
		glfwTerminate();                                                       \
		exit(-1);                                                              \
	}

#define INIT_GLEW(window)                                                      \
	glewExperimental = GL_TRUE;                                                \
	if (glewInit() != GLEW_OK) {                                               \
		glfwDestroyWindow(window);                                             \
		glfwTerminate();                                                       \
		exit(-1);                                                              \
	}

#define LOAD_MESH_OR_EXIT(mesh, file)                                          \
	if (!(mesh).LoadFromFileObj(file)) {                                       \
		std::cerr << "failed to load object: '" << file << "'\n";              \
		exit(-1);                                                              \
	}

#define LOG_SEVERITY(severity)                                                 \
	(severity == GL_DEBUG_SEVERITY_HIGH                                        \
		 ? "high"                                                              \
		 : (severity == GL_DEBUG_SEVERITY_MEDIUM                               \
				? "med"                                                        \
				: (severity == GL_DEBUG_SEVERITY_LOW ? "low" : "noti")))

inline void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
								   GLenum severity, GLsizei length,
								   const GLchar *message,
								   const void *userParam) {

	std::cerr << "[" << LOG_SEVERITY(severity) << "] OpenGL Debug"
			  << ": " << message << std::endl;
}

#define SETUP_DEBUG_CALLBACKS                                                  \
	glEnable(GL_DEBUG_OUTPUT);                                                 \
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);                                     \
	glDebugMessageCallback(DebugCallback, nullptr);                            \
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,         \
						  nullptr, GL_TRUE)

#endif
