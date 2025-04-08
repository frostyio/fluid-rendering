#ifndef _RENDERER_COMPONENT_H_
#define _RENDERER_COMPONENT_H_

#include "components/component.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"

using namespace engine;

namespace engine {

class RendererComponent : public Component {
  public:
	virtual void Render(Renderer &renderer, Scene *scene) = 0;
	virtual void Update() = 0;
};

} // namespace engine

#endif
