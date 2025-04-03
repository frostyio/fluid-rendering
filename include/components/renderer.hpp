#ifndef _RENDERER_COMPONENT_H_
#define _RENDERER_COMPONENT_H_

#include "components/component.hpp"
#include "core/renderer.hpp"

using namespace engine;

namespace engine {

class RendererComponent : public Component {
  public:
	virtual void Render(Renderer &renderer) = 0;
	virtual void Update() = 0;
};

} // namespace engine

#endif
