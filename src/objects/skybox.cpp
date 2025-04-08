#include "objects/skybox.hpp"
#include "common/meshUtil.h"
#include "objects/camera.hpp"

using namespace engine;

// full screen triangle
const float skyboxVertices[] = {-1.0f, -1.0f, 1.0f, 3.0f, -1.0f,
								1.0f,  -1.0f, 3.0f, 1.0f};

SkyboxObject::SkyboxObject() {
	skybox.Initialize();
	const static std::string sides[] = {"posx", "negx", "posy",
										"negy", "posz", "negz"};

	for (int i = 0; i < 6; i++) {
		std::string side = sides[i];

		std::vector<unsigned char> image;
		unsigned int image_width, image_height;
		loadImage(image, image_width, image_height,
				  "assets/textures/custom/" + side + ".png");

		skybox.SetImageRGBA((cy::GLTextureCubeMapSide)i, image.data(),
							image_width, image_height);
	}
	skybox.BuildMipmaps();
	skybox.SetSeamless();

	skyboxProg.BuildFiles("assets/shaders/skybox.vert",
						  "assets/shaders/skybox.frag");
	skyboxProg["skybox"] = 0;

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
				 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
						  (void *)0);
	glBindVertexArray(0);
}

void SkyboxObject::Render(Renderer &renderer, Scene *scene) {
	glClear(GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	skyboxProg.Bind();
	skybox.Bind(0);

	cy::Matrix4f skyboxViewMatrix = scene->GetActiveCamera()->GetView();
	skyboxViewMatrix.SetColumn(3, cy::Vec4f(0, 0, 0, 1));
	skyboxProg["invViewProj"] =
		(scene->GetActiveCamera()->GetProjection() * skyboxViewMatrix)
			.GetInverse();

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	renderer.BindProgram("default");
}
