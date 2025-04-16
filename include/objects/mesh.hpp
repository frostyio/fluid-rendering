#ifndef _MESH_H_
#define _MESH_H_

#include "components/mesh_renderer.hpp"
#include "core/scene.hpp"
#include "core/scene_object.hpp"
#include <memory>

namespace engine {

class MeshObject : public SceneObject {
  private:
	std::unique_ptr<MeshRendererComponent> mesh;
	Vec3f color = {1, 1, 1};
	float shininess = 100;

  public:
	MeshObject();
	MeshObject(cy::TriMesh &mesh);
	MeshObject(const std::vector<Vertex> &, const std::vector<unsigned int> &);
	MeshObject(const Vec3f &siz);

	inline void SetPosition(const Vec3f &pos) {
		position = pos;
		RefreshState();
	}
	inline void SetSize(const Vec3f &siz) {
		size = siz;
		RefreshState();
	}
	inline void SetRotation(const Quatf &rot) {
		rotation = rot;
		RefreshState();
	}
	inline void SetMeshSize(const Vec3f &size) { mesh->SetMeshSize(size); }
	inline void SetMeshColor(const Vec3f &c) { color = c; }
	inline void SetShininess(const float &f) { shininess = f; }

	inline void SetTextures(const std::string &path) {
		mesh->SetTextures(path);
	}

	void Render(Renderer &renderer, Scene *scene) override;
};

} // namespace engine

#endif
