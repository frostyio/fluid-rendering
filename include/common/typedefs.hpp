#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#include "cyMatrix.h"
#include "cyQuat.h"
#include "cyVector.h"

#include <GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>

#include "cyGL.h"

namespace engine {

using cy::Matrix4f, cy::Quatf, cy::Vec3f, cy::GLSLProgram;

}

#endif
