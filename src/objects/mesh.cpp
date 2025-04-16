#include "objects/mesh.hpp"
#include "components/mesh_renderer.hpp"
#include "core/scene_object.hpp"

MeshObject::MeshObject() {
	mesh = std::make_unique<MeshRendererComponent>();
	AddComponent(mesh.get());
};

MeshObject::MeshObject(cy::TriMesh &m) {
	mesh = std::make_unique<MeshRendererComponent>(m);
	AddComponent(mesh.get());
}

MeshObject::MeshObject(const std::vector<Vertex> &vertices,
					   const std::vector<unsigned int> &indices) {
	mesh = std::make_unique<MeshRendererComponent>(vertices, indices);
	AddComponent(mesh.get());
}

MeshObject::MeshObject(const Vec3f &siz) {
	const float w = siz.x * 0.5f;
	const float h = siz.y * 0.5f;
	const float d = siz.z * 0.5f;

	const std::vector<Vertex> CUBOID_VERTICES = {
		// Front face
		{{-w, -h, d}, {0, 0, 1}, {0, 0, 0}},
		{{w, -h, d}, {0, 0, 1}, {1, 0, 0}},
		{{w, h, d}, {0, 0, 1}, {1, 1, 0}},
		{{-w, h, d}, {0, 0, 1}, {0, 1, 0}},
		// Back face
		{{-w, -h, -d}, {0, 0, -1}, {1, 0, 0}},
		{{-w, h, -d}, {0, 0, -1}, {1, 1, 0}},
		{{w, h, -d}, {0, 0, -1}, {0, 1, 0}},
		{{w, -h, -d}, {0, 0, -1}, {0, 0, 0}},
		// Left face
		{{-w, -h, -d}, {-1, 0, 0}, {0, 0, 0}},
		{{-w, -h, d}, {-1, 0, 0}, {1, 0, 0}},
		{{-w, h, d}, {-1, 0, 0}, {1, 1, 0}},
		{{-w, h, -d}, {-1, 0, 0}, {0, 1, 0}},
		// Right face
		{{w, -h, -d}, {1, 0, 0}, {1, 0, 0}},
		{{w, h, -d}, {1, 0, 0}, {1, 1, 0}},
		{{w, h, d}, {1, 0, 0}, {0, 1, 0}},
		{{w, -h, d}, {1, 0, 0}, {0, 0, 0}},
		// Top face
		{{-w, h, d}, {0, 1, 0}, {0, 0, 0}},
		{{w, h, d}, {0, 1, 0}, {1, 0, 0}},
		{{w, h, -d}, {0, 1, 0}, {1, 1, 0}},
		{{-w, h, -d}, {0, 1, 0}, {0, 1, 0}},
		// Bottom face
		{{-w, -h, d}, {0, -1, 0}, {0, 0, 0}},
		{{-w, -h, -d}, {0, -1, 0}, {0, 1, 0}},
		{{w, -h, -d}, {0, -1, 0}, {1, 1, 0}},
		{{w, -h, d}, {0, -1, 0}, {1, 0, 0}},
	};

	const std::vector<unsigned int> CUBOID_INDICES = {// Front
													  0, 1, 2, 2, 3, 0,
													  // Back
													  4, 5, 6, 6, 7, 4,
													  // Left
													  8, 9, 10, 10, 11, 8,
													  // Right
													  12, 13, 14, 14, 15, 12,
													  // Top
													  16, 17, 18, 18, 19, 16,
													  // Bottom
													  20, 21, 22, 22, 23, 20};

	mesh = std::make_unique<MeshRendererComponent>(CUBOID_VERTICES,
												   CUBOID_INDICES);
	AddComponent(mesh.get());
}

void MeshObject::Render(Renderer &renderer, Scene *scene) {
	renderer.SetUniform("ambientColor", color);
	renderer.SetUniform("diffuseColor", color);
	renderer.SetUniform("shininess", shininess);
	SceneObject::Render(renderer, scene);
}
