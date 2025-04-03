#include "core/scene.hpp"
#include "core/renderer.hpp"
#include "objects/camera.hpp"

Scene::Scene() {}
Scene::~Scene() = default;

void Scene::AddObject(std::unique_ptr<SceneObject> object) {
	objects_.push_back(std::move(object));
}

void Scene::RemoveObject(SceneObject *object) {
	objects_.erase(
		std::remove_if(objects_.begin(), objects_.end(),
					   [object](const std::unique_ptr<SceneObject> &obj) {
						   return obj.get() == object;
					   }),
		objects_.end());
}

void Scene::Update(float deltaTime) {
	for (const auto &object : objects_) {
		object->Update(deltaTime);
	}
}

void Scene::Render(Renderer &renderer) {
	// Matrix4f viewMatrix = activeCamera_->GetViewMatrix();
	// Matrix4f projectionMatrix = activeCamera_->GetProjectionMatrix();
	// Renderer *renderer =
	// 	GetRendererInstance(); // Replace with your actual renderer access

	renderer.BindProgram("default");

	engine::CameraObject *camera = GetActiveCamera();
	renderer.SetView(camera->GetView());
	renderer.SetProjection(camera->GetProjection());

	renderer.SetShadingType(ShadingType::BlinnPhong);
	renderer.SetShaderUniforms(GetSunPosition(), camera->GetPosition());

	for (const auto &object : objects_) {
		object->Render(renderer);
	}
}
