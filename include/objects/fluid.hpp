#ifndef _FLUID_OBJECT_H_
#define _FLUID_OBJECT_H_

#include "components/fluid_simulation.hpp"
#include "core/scene_object.hpp"
#include <memory>

namespace engine {

class FluidObject : public SceneObject {
  private:
	std::unique_ptr<FluidData> fluid;
	Vec3f color = {0, 0, 1};

  public:
	FluidObject() {};

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

	void Render(Renderer &renderer) override;

	bool fromFile(const std::string &path);
	bool fromSimulation();
};

} // namespace engine

#endif
