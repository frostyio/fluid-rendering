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
	FluidObject() { renderingOrder = RenderingOrder::Post; };

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

	inline Vec3f &GetColor() const;

	void Render(Renderer &renderer, Scene *scene) override;

	bool fromFile(const std::string &path);
	bool fromFrameData(const std::vector<Vec3f> &, const size_t &numPoints,
					   const size_t &numFrames);
	bool fromSimulation();

	bool IsFinished() { return fluid->IsFinished(); }
	void Reset() { fluid->Reset(); }
};

} // namespace engine

#endif
