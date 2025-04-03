#include "common/typedefs.hpp"
#include "core/renderer.hpp"
#include "core/scene.hpp"
#include "core/scene_object.hpp"
#include "objects/camera.hpp"
#include "objects/mesh.hpp"
#include <memory>
#include <winscard.h>

int width, height;

//

static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
						int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static cy::Vec2<double> mouseChange(0.0, 0.0);
static cy::Vec2<double> lastMousePos(0.0, 0.0);
static bool isMouse1Pressed = false;
static bool isMouse2Pressed = false;
static cy::Vec2<double> accumulatedDrag(0., 0.);
static double accumulatedZoom = 45.;

static void mouseCallback(GLFWwindow *window, int button, int action,
						  int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2) {
		if (action == GLFW_PRESS) {
			double xPos, yPos;
			glfwGetCursorPos(window, &xPos, &yPos);
			lastMousePos = cy::Vec2(xPos, yPos);
			if (button == GLFW_MOUSE_BUTTON_1)
				isMouse1Pressed = true;
			else
				isMouse2Pressed = true;
		} else if (action == GLFW_RELEASE) {
			mouseChange = cy::Vec2(0.0, 0.0);
			if (button == GLFW_MOUSE_BUTTON_1)
				isMouse1Pressed = false;
			else
				isMouse2Pressed = false;
		}
	}
}

cy::Vec2<double> getMouseDelta(GLFWwindow *window) {
	if (isMouse1Pressed || isMouse2Pressed) {
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		cy::Vec2<double> currentMousePos(xPos, yPos);

		mouseChange = currentMousePos - lastMousePos;
		lastMousePos = currentMousePos;
		return mouseChange;
	}

	return cy::Vec2(0.0, 0.0);
}

Quatf FromAxisAngle(const Vec3f &axis, float angle_rad) {
	float half_angle = angle_rad * 0.5f;
	float s = sinf(half_angle);
	return {cosf(half_angle), axis.x * s, axis.y * s, axis.z * s};
}

int main(int argc, char **argv) {
	GLFW_SETUP;

#ifdef PROJECT_NAME
	CREATE_GLFW_WINDOW(640, 480, PROJECT_NAME);
#else
	exit(-1);
#endif

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// AFTER OpenGL context is created!
	INIT_GLEW(window);

	// callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	CY_GL_REGISTER_DEBUG_CALLBACK;
	SETUP_DEBUG_CALLBACKS;

	// opengl
	glfwGetFramebufferSize(window, &width, &height);

	//
	engine::Renderer renderer = engine::Renderer(width, height);
	engine::Scene *scene = new engine::Scene();
	scene->SetSunPosition({20, 10, 20});

	// renderer setup
	GLSLProgram prog;
	prog.BuildFiles("assets/shaders/shader.vert",
					"assets/shaders/shading.frag");
	renderer.CreateProgram("default", &prog);
	renderer.GetProgram("default");

	// camera
	engine::CameraObject *camera = new engine::CameraObject({0, 0, 0});
	scene->AddObject(std::unique_ptr<engine::SceneObject>(camera));
	scene->SetActiveCamera(camera);
	camera->SetAspectRatio((float)width / (float)height);

	//

	// TEAPOT OBJECT
	{
		cy::TriMesh mesh;
		LOAD_MESH_OR_EXIT(mesh, "assets/models/teapot.obj");
		engine::MeshObject *meshObject = new engine::MeshObject(mesh);
		meshObject->SetMeshColor({.4, 0.02, 0.02});
		scene->AddObject(std::unique_ptr<engine::SceneObject>(meshObject));
		Quatf rot = FromAxisAngle({1, 0, 0}, deg2rad(-90));
		meshObject->SetRotation(rot);
	}
	// SPHERE LIGHT OBJECT
	{
		cy::TriMesh mesh;
		LOAD_MESH_OR_EXIT(mesh, "assets/models/sphere.obj");
		engine::MeshObject *meshObject = new engine::MeshObject(mesh);
		meshObject->SetPosition(scene->GetSunPosition());
		meshObject->SetMeshSize({0.3, 0.3, 0.3});
		meshObject->SetMeshColor({1, 1, 0});
		scene->AddObject(std::unique_ptr<engine::SceneObject>(meshObject));
	}
	// PLANE
	{
		const std::vector<Vertex> PLANE_VERTICES = {
			{{-10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}},
			{{10.0, 0.0, -10.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0}},
			{{10.0, 0.0, 10.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}},
			{{-10.0, 0.0, 10.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 0.0}}};
		const std::vector<unsigned int> PLANE_INDICES = {0, 1, 2, 2, 3, 0};
		engine::MeshObject *meshObject =
			new engine::MeshObject(PLANE_VERTICES, PLANE_INDICES);
		meshObject->SetPosition({0, -10, 0});
		meshObject->SetMeshSize({5, 5, 5});
		meshObject->SetMeshColor({.2, .2, .2});
		scene->AddObject(std::unique_ptr<engine::SceneObject>(meshObject));
	}

	renderer.BindProgram("default");

	while (!glfwWindowShouldClose(window)) {
		renderer.BeginFrame();

		if (isMouse1Pressed)
			accumulatedDrag += getMouseDelta(window) * 0.3;
		if (isMouse2Pressed)
			accumulatedZoom += getMouseDelta(window).y * 0.05;
		float theta = deg2rad(accumulatedDrag.x);
		float phi = deg2rad(accumulatedDrag.y);
		float radius = accumulatedZoom;
		cy::Vec3f cameraPos =
			cy::Vec3f{radius * cos(phi) * sin(theta), radius * sin(phi),
					  radius * cos(phi) * cos(theta)};
		camera->SetPosition(cameraPos);

		scene->Update(0.0);
		scene->Render(renderer);

		renderer.EndFrame(window);

		glfwPollEvents();
	}

	delete scene;
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
