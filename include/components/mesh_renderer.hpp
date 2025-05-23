#ifndef _MESH_RENDERER_COMPONENT_H_
#define _MESH_RENDERER_COMPONENT_H_

#include "common/common.hpp"
#include "common/typedefs.hpp"
#include "components/renderer.hpp"
#include "core/renderer.hpp"

using namespace engine;

namespace engine {

class MeshRendererComponent : public RendererComponent {
  private:
	GLuint VAO, VBO, EBO;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	bool hasSentData = false;

	bool hasDiffuse = false, hasNormal = false, hasRough = false,
		 hasDisp = false;
	// GLuint diffuseTex, normalTex, roughTex, dispTex;
	cyGLTexture2D diffuseTex, normalTex, roughTex, dispTex;

  protected:
	Vec3f meshSize = Vec3f{};
	Vec3f center = Vec3f{};
	Matrix4f modelMatrix;

	void UpdateModelMatrix();

	void Bind(Renderer &renderer);
	void SendData(Renderer &renderer);

  public:
	MeshRendererComponent();
	MeshRendererComponent(cy::TriMesh &mesh);
	MeshRendererComponent(const std::vector<Vertex> &,
						  const std::vector<unsigned int> &);

	inline unsigned int NV() { return indices.size(); }
	inline Vec3f GetCenter() const { return center; }
	inline Vec3f GetMeshSize() const { return meshSize; }
	inline Matrix4f GetModelMatrix() const { return modelMatrix; }

	void SetMeshSize(const Vec3f &size);
	void SetTextures(const std::string path);

	void Render(Renderer &renderer, Scene *scene) override;
	void Update() override;
};

} // namespace engine

#endif
