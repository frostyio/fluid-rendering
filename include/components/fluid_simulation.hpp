#ifndef _FLUID_SIMULATION_H_
#define _FLUID_SIMULATION_H_

#include "core/renderer.hpp"
#include "core/scene.hpp"
#include <optional>

#undef max
#undef min

#include <Alembic/Abc/ICompoundProperty.h>
#include <Alembic/Abc/IObject.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcGeom/IPoints.h>

#include "common/common.hpp"
#include "components/component.hpp"

using namespace Alembic::AbcCoreFactory;

namespace engine {

class FluidData : public Component, public IUpdatable {
  public:
	virtual ~FluidData() = default;
	virtual void Bind() = 0;
	virtual void Draw(Renderer &renderer, Scene *scene, Matrix4f model) = 0;
};

class BakedPointDataComponent : public FluidData {
  private:
	GLuint vao, buffer;
	GLuint depthFBOA, depthTextureA;
	GLuint depthFBOB, depthTextureB;
	GLuint filteredDepthTexture, filterFBO;
	GLuint normalFBO, normalTexture;
	GLuint thicknessFBO, thicknessTexture;

	size_t currentFrame, numPoints, numFrames;
	float timer = 0;

  public:
	BakedPointDataComponent(const Alembic::Abc::IArchive &archive);
	static std::optional<BakedPointDataComponent>
	create(const std::string &path);

	void Bind() override;
	void Update(double dt) override;
	void Draw(Renderer &renderer, Scene *scene, Matrix4f model) override;
};

class FluidSimulationComponent : public FluidData {
  private:
	GLuint buffer;

  public:
	void Bind() override;
	void Update(double dt) override;
	void Draw(Renderer &renderer, Scene *scene, Matrix4f model) override;
};

} // namespace engine

#endif
