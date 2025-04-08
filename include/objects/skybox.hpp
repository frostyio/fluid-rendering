#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "core/scene.hpp"
#include "core/scene_object.hpp"

namespace engine {

class SkyboxObject : public SceneObject {
  private:
	GLuint skyboxVAO, skyboxVBO;
	cy::GLTextureCubeMap skybox;
	cy::GLSLProgram skyboxProg;

  public:
	SkyboxObject();

	inline cy::GLTextureCubeMap &GetTexture() { return skybox; }

	void Render(Renderer &renderer, Scene *scene) override;
};

} // namespace engine

#endif
