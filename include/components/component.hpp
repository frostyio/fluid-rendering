#ifndef _COMPONENT_H_
#define _COMPONENT_H_

namespace engine {

class SceneObject;

class Component {
  public:
	Component() : owner_(nullptr) {}
	virtual ~Component() = default;

	SceneObject *GetOwner() const { return owner_; }

	void SetOwner(SceneObject *owner) { owner_ = owner; }

  protected:
	SceneObject *owner_;
};

class IUpdatable {
  public:
	virtual void Update(double deltaTime) = 0;
	virtual ~IUpdatable() = default;
};

} // namespace engine

#endif
