#ifndef _MESH_UTIL_H_
#define _MESH_UTIL_H_

#include "common/common.hpp"
#include "cyTriMesh.h"

void preprocessOBJ(const cy::TriMesh &mesh, std::vector<engine::Vertex> &outV,
				   std::vector<unsigned int> &outI);

bool loadImage(std::vector<unsigned char> &image, unsigned &width,
			   unsigned &height, std::string path);

bool loadTexture(cyGLTexture2D &tex, std::string path);

#endif
