#include "objects/camera.hpp"
#include "common/util.hpp"

using engine::CameraObject;

void CameraObject::UpdateState() {
	float fov_v_rad = 2.0f * std::atan(std::tan(deg2rad(fov) / 2.0f) / aspect);
	projMatrix = cy::Matrix4f::Perspective(rad2deg(fov_v_rad), aspect,
										   nearPlane, farPlane);
	viewMatrix =
		cy::Matrix4f::View(position, cy::Vec3f(0, 0, 0), cy::Vec3f(0, 1, 0));
}
