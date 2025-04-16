#include "objects/fluid.hpp"
#include "components/fluid_simulation.hpp"
#include "core/renderer.hpp"

using namespace engine;

bool FluidObject::fromFile(const std::string &path) {
	auto data = BakedPointDataComponent::create(path);
	if (!data)
		return false;

	fluid = std::make_unique<BakedPointDataComponent>(std::move(data.value()));
	AddComponent(fluid.get());
	return true;
}

bool FluidObject::fromFrameData(const std::vector<Vec3f> &frameData,
								const size_t &nPoints, const size_t &nFrames) {
	fluid = std::make_unique<BakedPointDataComponent>(std::move(frameData),
													  nPoints, nFrames);
	AddComponent(fluid.get());
	return true;
}

void FluidObject::Render(Renderer &renderer, Scene *scene) {
	fluid->Bind();
	renderer.SetUniform("ambientColor", color);
	renderer.SetShadingType(ShadingType::SolidAmbient);
	Matrix4f modelMatrix = Matrix4f::Translation(position) *
						   rotation.ToMatrix4() * Matrix4f::Scale(size);
	renderer.SetUniform("model", modelMatrix);
	renderer.DrawMesh();
	fluid->Draw(renderer, scene, modelMatrix);
	SceneObject::Render(renderer, scene);
}
