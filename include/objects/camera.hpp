#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "core/scene_object.hpp"

namespace engine {

class CameraObject : public SceneObject {
  private:
	float fov = 90;
	float aspect = 1920. / 1080.;
	float nearPlane = 0.01;
	float farPlane = 1000;

	Matrix4f viewMatrix;
	Matrix4f projMatrix;

  public:
	CameraObject();
	CameraObject(const Vec3f &pos) {
		position = pos;
		UpdateState();
	}
	CameraObject(const Vec3f &pos, const Quatf &rot) {
		position = pos;
		rotation = rot;
		UpdateState();
	}
	CameraObject(const Vec3f &pos, const Quatf &rot, float fov, float aspect,
				 float nearPlane, float farPlane)
		: fov(fov), aspect(aspect), nearPlane(nearPlane), farPlane(farPlane) {
		position = pos;
		rotation = rot;
		UpdateState();
	};

	inline void SetPosition(const Vec3f &pos) {
		position = pos;
		UpdateState();
	}
	inline void SetSize(const Vec3f &siz) {
		size = siz;
		UpdateState();
	}
	inline void SetRotation(const Quatf &rot) {
		rotation = rot;
		UpdateState();
	}
	inline void SetAspectRatio(const float ratio) {
		aspect = ratio;
		UpdateState();
	}

	inline Matrix4f GetView() const { return viewMatrix; }
	inline Matrix4f GetProjection() const { return projMatrix; }
	void UpdateState();

	inline float GetFov() const { return fov; }
};

} // namespace engine

#endif
