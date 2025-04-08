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

void MeshObject::Render(Renderer &renderer, Scene *scene) {
	renderer.SetUniform("ambientColor", color);
	renderer.SetUniform("diffuseColor", color);
	renderer.SetUniform("shininess", shininess);
	SceneObject::Render(renderer, scene);
}
