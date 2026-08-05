#pragma once
#include <glad/glad.h>
#include <glfw3.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "camera_ver2.h"

class Skybox
{
public:

	unsigned int skyboxVAO;
	unsigned int skyboxVBO;
	unsigned int cubemapTexture;

	Skybox();
	Skybox(unsigned int& SkyboxVAO, unsigned int& SkyboxVBO, unsigned int& CubemapTexture);
	void SkyboxInit();
	void SkyboxRender(Shader& spaceboxShader, Camera_ver2& camera, const glm::mat4& projection);

};

