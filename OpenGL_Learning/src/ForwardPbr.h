#pragma once
#include "PbrModel.h"
#include "Shader.h"

struct ForwardSceneInfo
{
	glm::mat4 projection, view;	// 相机（projection / view）
	glm::vec3 camPos;			// 相机位置

	// 恒星
	glm::vec3 sunPos;
	glm::vec3 sunColor;
	float sunConstant;
	float sunLinear;
	float sunQuadratic;

	// IBL
	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUT;

	// 阴影
	unsigned int depthCubeMap;
	float shadowFar;
	bool shadows;
	bool PCSS;
};

struct ForwardPbrMaterial
{
	bool  useMetallicMap = true;
	bool  useRoughnessMap = true;
	bool  useEmissiveMap = false;
	bool  useAOMap = false;
	float metallicValue = 0.0f;
	float roughnessValue = 1.0f;
	float aoValue = 1.0f;
	float emissiveStrength = 1.0f;
	glm::vec3 rimColor = glm::vec3(0.4f, 0.6f, 1.0f);
	float rimPower = 3.0f;
	float rimStrength = 0.0f;
};

class ForwardPbr
{
public:
	
	PbrModel model;
	
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale	   = glm::vec3(1.0f);
	ForwardPbrMaterial material;

	ForwardPbr(const std::string& modelPath) :model(modelPath)
	{
	}

	void Draw(const ForwardSceneInfo& Info, Shader& shader);
	void DrawDepth(Shader& shader);

private:
	
	glm::mat4 GetModelMatrix() const;
};

