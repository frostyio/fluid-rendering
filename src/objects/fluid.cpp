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

void FluidObject::Render(Renderer &renderer) {
	fluid->Bind();
	renderer.SetUniform("ambientColor", color);
	renderer.SetShadingType(ShadingType::SolidAmbient);
	renderer.SetUniform("model", Matrix4f::Translation(position) *
									 rotation.ToMatrix4() *
									 Matrix4f::Scale(size));
	renderer.DrawMesh();
	fluid->Draw();
	SceneObject::Render(renderer);
}
