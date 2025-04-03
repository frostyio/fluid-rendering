#ifndef _SCENE_H_
#define _SCENE_H_

#include "core/renderer.hpp"
#include "core/scene_object.hpp"
#include "objects/camera.hpp"
#include <memory>
#include <vector>

using namespace engine;

namespace engine {

class Scene {
  private:
	std::vector<std::unique_ptr<SceneObject>> objects_;
	CameraObject *activeCamera = nullptr;

	Vec3f sunPosition = Vec3f{0., 0., 0.};

  public:
	Scene();
	~Scene();

	void AddObject(std::unique_ptr<SceneObject> object);
	void RemoveObject(SceneObject *object);
	const std::vector<std::unique_ptr<SceneObject>> &GetObjects() const {
		return objects_;
	}

	void Update(float deltaTime);
	void Render(Renderer &renderer);

	inline void SetActiveCamera(CameraObject *camera) { activeCamera = camera; }
	inline CameraObject *GetActiveCamera() { return activeCamera; }

	inline void SetSunPosition(const Vec3f &pos) { sunPosition = pos; }
	inline const Vec3f &GetSunPosition() { return sunPosition; }
};

} // namespace engine

#endif
