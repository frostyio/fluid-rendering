#include "core/scene.hpp"
#include "core/renderer.hpp"
#include "core/scene_object.hpp"
#include "objects/camera.hpp"
#include "objects/skybox.hpp"

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

void RenderObjects(Scene *scene, Renderer &renderer,
				   std::vector<SceneObject *> objects) {
	renderer.BindProgram("default");
	engine::CameraObject *camera = scene->GetActiveCamera();
	renderer.SetView(camera->GetView());
	renderer.SetProjection(camera->GetProjection());

	renderer.SetShadingType(ShadingType::BlinnPhong);
	renderer.SetShaderUniforms(scene->GetSunPosition(), camera->GetPosition());

	for (const auto &object : objects) {
		renderer.BindProgram("default");
		renderer.SetDummyTextures();
		object->Render(renderer, scene);
	}
}

void Scene::RenderOpaque(Renderer &renderer, std::vector<SceneObject *> objects,
						 std::string buffer) {
	renderer.BindBuffer(buffer);
	RenderObjects(this, renderer, objects);
}
void Scene::RenderTransparent(Renderer &renderer,
							  std::vector<SceneObject *> objects,
							  std::string buffer) {
	renderer.BindBuffer(buffer);
	glEnable(GL_BLEND);
	RenderObjects(this, renderer, objects);
}
void Scene::RenderPost(Renderer &renderer, std::vector<SceneObject *> objects,
					   std::string buffer) {
	renderer.BindBuffer(buffer);
	glEnable(GL_BLEND);
	renderer.BeginFrame();
	RenderObjects(this, renderer, objects);
}

void Scene::Render(Renderer &renderer) {
	std::vector<SceneObject *> opaqueObjects = {};
	std::vector<SceneObject *> transObjects = {};
	std::vector<SceneObject *> postObjects = {};

	for (const auto &object : objects_) {
		if (static_cast<void *>(object.get()) ==
			static_cast<void *>(activeSkybox)) {
			continue;
		}

		switch (object->GetRenderingOrder()) {
		case engine::RenderingOrder::Opaque:
			opaqueObjects.push_back(object.get());
			break;
		case engine::RenderingOrder::Transparent:
			transObjects.push_back(object.get());
			break;
		case engine::RenderingOrder::Post:
			postObjects.push_back(object.get());
			break;
		}
	}

	renderer.BindBuffer("opaque");
	renderer.BeginFrame();

	renderer.BindProgram("default");
	engine::CameraObject *camera = GetActiveCamera();
	renderer.SetView(camera->GetView());
	renderer.SetProjection(camera->GetProjection());

	renderer.SetShadingType(ShadingType::BlinnPhong);
	renderer.SetShaderUniforms(GetSunPosition(), camera->GetPosition());
	activeSkybox->Render(renderer, this);

	RenderOpaque(renderer, opaqueObjects);
	RenderTransparent(renderer, transObjects);
	RenderPost(renderer, postObjects);

	renderer.Composite();
}
