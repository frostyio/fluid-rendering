#ifndef _SCENE_OBJECT_H_
#define _SCENE_OBJECT_H_

#include "common/typedefs.hpp"
#include "components/renderer.hpp"
#include "core/renderer.hpp"
#include <vector>

using namespace engine;

namespace engine {

class Component;

class SceneObject {
  protected:
	Vec3f position = Vec3f{};
	Quatf rotation = Quatf{1, 0, 0, 0};
	Vec3f size = Vec3f{1, 1, 1};

  public:
	virtual ~SceneObject() = default;
	template <typename T> T *GetComponent() const {
		for (Component *comp : components) {
			if (dynamic_cast<T *>(comp)) {
				return static_cast<T *>(comp);
			}
		}
		return nullptr;
	}

	template <typename T> void AddComponent(T *component) {
		component->SetOwner(this);
		components.push_back(component);
		if (RendererComponent *renderComp =
				dynamic_cast<RendererComponent *>(component)) {
			renderComponents.push_back(renderComp);
		}
	}

	template <typename T> void RemoveComponent() {
		for (auto it = components.begin(); it != components.end(); ++it) {
			if (T *comp = dynamic_cast<T *>(*it)) {
				if (RendererComponent *renderComp =
						dynamic_cast<RendererComponent *>(*it)) {
					for (auto rit = renderComponents.begin();
						 rit != renderComponents.end(); ++rit) {
						if (*rit == renderComp) {
							renderComponents.erase(rit);
							break;
						}
					}
				}
				delete comp;
				components.erase(it);
				return;
			}
		}
	}

	inline Vec3f GetPosition() const { return position; }
	inline Quatf GetRotation() const { return rotation; }
	inline Vec3f GetSize() const { return size; }

	virtual void Update(double deltaTime) {
		for (RendererComponent *renderComp : renderComponents) {
			renderComp->Update();
		}

		for (Component *comp : components) {
			if (auto *updatable = dynamic_cast<IUpdatable *>(comp)) {
				updatable->Update(deltaTime);
			}
		}
	}

	virtual void RefreshState() {
		for (RendererComponent *renderComp : renderComponents) {
			renderComp->Update();
		}
	}

	virtual void Render(Renderer &renderer) {
		for (RendererComponent *renderComp : renderComponents) {
			renderComp->Render(renderer);
		}
	}

  protected:
	std::vector<Component *> components;
	std::vector<RendererComponent *> renderComponents;
};

} // namespace engine

#endif
