#include "common/meshUtil.h"
#include "lodepng.h"
#include <iostream>
#include <unordered_map>

using engine::Vertex;

void preprocessOBJ(const cy::TriMesh &mesh, std::vector<Vertex> &outV,
				   std::vector<unsigned int> &outI) {
	std::unordered_map<Vertex, unsigned int> map;

	using Face = cy::TriMesh::TriFace;
	for (size_t i = 0; i < mesh.NF(); i++) {
		const Face vertexFace = mesh.F(i);
		const Face normalFace = mesh.FN(i);
		const Face textureFace = mesh.FT(i);

		for (int j = 0; j < 3; j++) {
			Vertex vertex;
			vertex.position = mesh.V(vertexFace.v[j]);
			vertex.normal = mesh.VN(normalFace.v[j]);
			vertex.texCoord = mesh.VT(textureFace.v[j]);

			auto it = map.find(vertex);
			if (it == map.end()) { // add new
				unsigned int newI = outV.size();
				map[vertex] = newI;
				outV.push_back(vertex);
				outI.push_back(newI);
			} else {
				outI.push_back(it->second);
			}
		}
	}
}

bool loadImage(std::vector<unsigned char> &image, unsigned &width,
			   unsigned &height, std::string path) {
	unsigned error = lodepng::decode(image, width, height, path);
	if (error) {
		std::cout << "failed to load texture at '" << path
				  << "' with error: " << lodepng_error_text(error) << std::endl;
		return false;
	}
	return true;
}

bool loadTexture(cyGLTexture2D &tex, std::string path) {
	std::vector<unsigned char> image;
	unsigned image_width, image_height;
	if (!loadImage(image, image_width, image_height, path))
		return false;

	tex.Initialize();
	tex.SetImage(image.data(), 4, image_width, image_height);
	tex.BuildMipmaps();

	return true;
}
