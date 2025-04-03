#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "common/typedefs.hpp"
#include <cstring>
#include <string>
#include <unordered_map>

namespace engine {

enum ShadingType {
	None = 0,
	BlinnPhong = 1,
};

class Renderer {
  private:
	std::unordered_map<std::string, GLSLProgram *> programs;
	std::string currentlyBinded = "";

	ShadingType shadingType = ShadingType::None;

  public:
	Renderer(int width, int height);

	void CreateProgram(std::string name, GLSLProgram *prog);
	GLSLProgram *GetProgram(std::string name);
	void BindProgram(std::string name);

	void BeginFrame();
	void EndFrame(GLFWwindow *window);

	template <typename T> void SetUniform(const char *key, T value) {
		if (currentlyBinded == "") {
			std::cout << "no program is currently bound" << std::endl;
			return;
		}

		GLSLProgram *prog = GetProgram(currentlyBinded);
		prog->SetUniform(key, value);
	}

	template <> void SetUniform<Matrix4f>(const char *key, Matrix4f value) {
		if (currentlyBinded == "") {
			std::cout << "no program is currently bound" << std::endl;
			return;
		}

		cyGLSLProgram *prog = GetProgram(currentlyBinded);
		if (std::strcmp(key, "model") == 0)
			SetModel(prog, value);
		else if (std::strcmp(key, "view") == 0)
			SetView(prog, value);
		else if (std::strcmp(key, "projection") == 0)
			SetProjection(prog, value);
		else
			std::cout << "other matrices uniform set not supported right now: "
					  << key << std::endl;
	}

	void SetModel(GLSLProgram *prog, Matrix4f m) {
		prog->SetUniform("model", m);
	}
	void SetModel(Matrix4f m) { SetUniform("model", m); }

	void SetView(GLSLProgram *prog, Matrix4f m) { prog->SetUniform("view", m); }
	void SetView(Matrix4f m) { SetUniform("view", m); }

	void SetProjection(GLSLProgram *prog, Matrix4f m) {
		prog->SetUniform("projection", m);
	}
	void SetProjection(Matrix4f m) { SetUniform("projection", m); }

	/**
	Sets respective shading uniform and prepares for draw call
	*/
	void DrawMesh();

	inline void SetShadingType(ShadingType t) { shadingType = t; };
	inline void SetShaderUniforms(Vec3f lightPosition, Vec3f cameraPosition) {
		SetUniform("lightPos", lightPosition);
		SetUniform("viewPos", cameraPosition);
	};
	inline void SetShaderUniforms(Vec3f lightPosition, Vec3f cameraPosition,
								  Vec3f ambientColor, Vec3f diffuseColor,
								  Vec3f specularColor, float shininess) {
		SetUniform("lightPos", lightPosition);
		SetUniform("viewPos", cameraPosition);
		SetUniform("ambientColor", ambientColor);
		SetUniform("diffuseColor", diffuseColor);
		SetUniform("specularColor", specularColor);
		SetUniform("shininess", shininess);
	};
};

} // namespace engine

#endif
