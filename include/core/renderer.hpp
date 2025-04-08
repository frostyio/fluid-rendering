#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "common/typedefs.hpp"
#include <string>
#include <unordered_map>

namespace engine {

enum ShadingType {
	None = 0,
	BlinnPhong = 1,
	SolidAmbient = 2,
};

inline GLint CurrentDrawFBO() {
	GLint currentFBO = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO);
	return currentFBO;
}

class Renderer {
  private:
	// RENDERER CONFIG
	int width, height;

	// STATE

	// programs
	std::unordered_map<std::string, GLSLProgram *> programs = {};
	std::string currentlyBinded = "";

	// uniform cache
	ShadingType shadingType = ShadingType::None;
	Matrix4f model, view, projection;

	// dummy uniform values
	struct SamplerInfo {
		std::string name;
		GLenum type;
		GLint location;
		GLint textureUnit;
	};

	std::unordered_map<GLuint, std::vector<SamplerInfo>> samplersByProgram;

	// dummy textures
	bool dummyTexturesInitialized;
	GLuint dummy2DTexture;
	GLuint dummyCubemapTexture;

	void CollectSamplerUniforms(GLuint programId);
	void InitializeDummyTextures();

	// buffers
	struct BufferInfo {
		GLuint id;
		GLuint texture;
		int width, height;
		GLuint depthTex;
	};
	std::unordered_map<std::string, BufferInfo> framebuffers = {};

	//
	GLuint fullscreenQuadVAO, fullscreenQuadVBO;

  public:
	Renderer(int width, int height);
	~Renderer();

	void CreateProgram(std::string name, GLSLProgram *prog);
	GLSLProgram *GetProgram(std::string name);
	void BindProgram(std::string name);

	void BindTexture(const char *name, GLuint textureID,
					 GLenum textureUnit = GL_TEXTURE0,
					 GLenum type = GL_TEXTURE_2D);

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

	void SetModel(GLSLProgram *prog, Matrix4f m) {
		prog->SetUniform("model", m);
		model = m;
	}
	void SetModel(Matrix4f m) { SetUniform("model", m); }

	void SetView(GLSLProgram *prog, Matrix4f m) {
		prog->SetUniform("view", m);
		view = m;
	}
	void SetView(Matrix4f m) { SetUniform("view", m); }

	void SetProjection(GLSLProgram *prog, Matrix4f m) {
		prog->SetUniform("projection", m);
		projection = m;
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

	inline Matrix4f GetModel() { return model; }
	inline Matrix4f GetView() { return view; }
	inline Matrix4f GetProjection() { return projection; }

	void SetDummyTextures();

	GLuint CreateDummyTexture2D();
	GLuint CreateDummyCubemap();

	/**
		Creates a framebuffer and a texture

		Returns texture id
	 */
	inline GLint CreateBuffer(std::string name, int width, int height,
							  GLenum attachment = GL_COLOR_ATTACHMENT0,
							  GLenum target = GL_TEXTURE_2D,
							  GLenum iFormat = GL_RGBA8,
							  GLenum format = GL_RGBA,
							  GLenum type = GL_UNSIGNED_BYTE,
							  bool withDepth = false) {
		GLint current = CurrentDrawFBO();

		GLuint fbo, tex;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &tex);
		glBindTexture(target, tex);
		glTexImage2D(target, 0, iFormat, width, height, 0, format, type,
					 nullptr);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, tex, 0);

		GLuint depthTex = 0;
		if (withDepth) {
			glGenTextures(1, &depthTex);
			glBindTexture(GL_TEXTURE_2D, depthTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height,
						 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
								   GL_TEXTURE_2D, depthTex, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, current);

		framebuffers[name] = {fbo, tex, width, height, depthTex};
		return tex;
	}

	/**
		Find a buffer

		Return buffer pointer
	*/
	BufferInfo *FindBuffer(const std::string &name) {
		auto it = framebuffers.find(name);
		if (it != framebuffers.end()) {
			return &it->second;
		} else {
			return nullptr;
		}
	}

	/**
		Bind a buffer if found

		Returns success
	*/
	inline bool BindBuffer(const std::string &name) {
		auto it = framebuffers.find(name);
		if (it != framebuffers.end()) {
			glBindFramebuffer(GL_FRAMEBUFFER, it->second.id);
			glViewport(0, 0, it->second.width, it->second.height);
			return true;
		} else {
			return false;
		}
	}
	inline bool BindBuffer(GLuint id) {
		for (const auto &[name, fb] : framebuffers) {
			if (fb.id == id) {
				glBindFramebuffer(GL_FRAMEBUFFER, fb.id);
				glViewport(0, 0, fb.width, fb.height);
				return true;
			}
		}
		return false;
	}

	/**
		Draws a fullscreen quad
	*/
	inline void DrawFullscreenQuad() {
		glBindVertexArray(fullscreenQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	/**
		Compose layers together
	 */
	void Composite();
};

} // namespace engine

#endif
