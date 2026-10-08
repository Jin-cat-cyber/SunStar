#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "PbrModel.h"


void SpaceShipWarpInit();

void warpShellPass(Shader& warpShellShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::mat4& spaceshipModel, const glm::vec3& camPos, float currentFrame, PbrModel& spaceship);

void warpGhostPass(Shader& warpGhostShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::mat4& shipWireModel, const glm::vec3& camPos, PbrModel& spaceship);

void warpWirePass(Shader& warpWireShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::mat4& shipWireModel, PbrModel& spaceship);

void warpPillarPass(Shader& warpDebrisShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::vec3& camPos, const glm::mat4& pillarPlane);

void warpDebrisPass(Shader& warpDebrisShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::vec3& camPos);

void warpShockPass(Shader& warpShockShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::vec3& camPos, const glm::mat4& axisModel);

void warpShockDistortPass(Shader& warpShockDistortShader, const glm::mat4& projection, const glm::mat4& view,
	const glm::mat4& axisModel, int windowwidth, int windowheight, unsigned int quadVAO);

void warpFlashPass(Shader& warpFlashShader, float flashAmount, float rejectFlash, const glm::mat4& projection,
	const glm::mat4& view, const glm::vec3& shipPos, unsigned int quadVAO);