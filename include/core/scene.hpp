#ifndef _SCENE_H_
#define _SCENE_H_

#include "core/renderer.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

using namespace engine;

namespace engine {

class SceneObject;
class CameraObject;
class SkyboxObject;

class Scene {
  private:
	std::vector<std::unique_ptr<SceneObject>> objects_;
	std::unordered_map<std::string, SceneObject *> named_objects_;
	CameraObject *activeCamera = nullptr;
	SkyboxObject *activeSkybox = nullptr;

	Vec3f sunPosition = Vec3f{0., 0., 0.};

  public:
	Scene();
	~Scene();

	void AddObject(std::unique_ptr<SceneObject> object);
	void AddObject(const std::string &name,
				   std::unique_ptr<SceneObject> object);
	void RemoveObject(SceneObject *object);
	const std::vector<std::unique_ptr<SceneObject>> &GetObjects() const {
		return objects_;
	}
	SceneObject *GetObject(const std::string &name) const {
		auto it = named_objects_.find(name);
		if (it != named_objects_.end()) {
			return it->second;
		}
		return nullptr;
	}

	inline void SetActiveCamera(CameraObject *camera) { activeCamera = camera; }
	inline CameraObject *GetActiveCamera() { return activeCamera; }

	inline void SetActiveSkybox(SkyboxObject *skybox) { activeSkybox = skybox; }
	inline SkyboxObject *GetActiveSkybox() { return activeSkybox; }

	inline void SetSunPosition(const Vec3f &pos) { sunPosition = pos; }
	inline const Vec3f &GetSunPosition() { return sunPosition; }

	void Update(float deltaTime);
	void Render(Renderer &renderer);

	void RenderOpaque(Renderer &renderer, std::vector<SceneObject *> objects,
					  std::string buffer = "opaque");
	void RenderTransparent(Renderer &renderer,
						   std::vector<SceneObject *> objects,
						   std::string buffer = "trans");
	void RenderPost(Renderer &renderer, std::vector<SceneObject *> objects,
					std::string buffer = "post");
};

} // namespace engine

#endif
