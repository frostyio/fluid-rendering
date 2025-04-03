#include "components/mesh_renderer.hpp"
#include "common/meshUtil.h"
#include "common/typedefs.hpp"
#include "core/scene_object.hpp"

void createVertexArrayAndBuffer(GLuint &VAO, GLuint &VBO, GLuint &EBO) {
	glGenVertexArrays(1, &VAO);

	GLuint buffers[2];
	glGenBuffers(2, buffers);

	VBO = buffers[0];
	EBO = buffers[1];
}

MeshRendererComponent::MeshRendererComponent() : meshSize({1, 1, 1}) {
	createVertexArrayAndBuffer(VAO, VBO, EBO);
	UpdateModelMatrix();
}

MeshRendererComponent::MeshRendererComponent(cy::TriMesh &mesh)
	: meshSize({1, 1, 1}) {
	createVertexArrayAndBuffer(VAO, VBO, EBO);

	mesh.ComputeBoundingBox();
	Vec3f boundMin = mesh.GetBoundMin();
	Vec3f boundMax = mesh.GetBoundMax();

	center = (boundMin + boundMax) * 0.5f;

	preprocessOBJ(mesh, vertices, indices);
	UpdateModelMatrix();
}

MeshRendererComponent::MeshRendererComponent(
	const std::vector<Vertex> &vertices,
	const std::vector<unsigned int> &indices)
	: vertices(vertices), indices(indices), meshSize({1, 1, 1}) {
	createVertexArrayAndBuffer(VAO, VBO, EBO);
	UpdateModelMatrix();
}

void MeshRendererComponent::Bind(Renderer &renderer) { glBindVertexArray(VAO); }

void MeshRendererComponent::SendData(Renderer &renderer) {
	if (hasSentData)
		return;
	hasSentData = true;

	this->Bind(renderer);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
				 vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
				 indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, texCoord));
}

void MeshRendererComponent::SetMeshSize(const Vec3f &size) {
	meshSize = size;
	UpdateModelMatrix();
}

void MeshRendererComponent::UpdateModelMatrix() {
	Matrix4f scaleMatrix = Matrix4f::Scale(meshSize);
	Matrix4f originMatrix = Matrix4f::Translation(-center);

	SceneObject *owner = GetOwner();
	if (owner != nullptr) {
		cy::Matrix4f scaleMatrix2 = cy::Matrix4f::Scale(owner->GetSize());
		cy::Matrix4f rotationMatrix = owner->GetRotation().ToMatrix4();
		cy::Matrix4f positionMatrix =
			cy::Matrix4f::Translation(owner->GetPosition());
		modelMatrix = positionMatrix * rotationMatrix * scaleMatrix2 *
					  scaleMatrix * originMatrix;
		// modelMatrix = positionMatrix * rotationMatrix * scaleMatrix2;
		// modelMatrix = positionMatrix;

	} else {
		modelMatrix = scaleMatrix * originMatrix;
	}
}

void MeshRendererComponent::Render(Renderer &renderer) {
	if (!hasSentData)
		SendData(renderer);

	Bind(renderer);
	renderer.SetUniform("model", modelMatrix);
	renderer.DrawMesh(); // set shading uniform
	glDrawElements(GL_TRIANGLES, NV(), GL_UNSIGNED_INT, 0);
}

void MeshRendererComponent::Update() { UpdateModelMatrix(); }
