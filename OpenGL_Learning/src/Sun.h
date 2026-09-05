#pragma once
#include <glad/glad.h>
#include <glfw3.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "camera_ver2.h"

class Sun
{
public:
    std::vector<float> starVertices;
    std::vector<unsigned int> starIndices;
    // 生成球体顶点数据（可以使用UV球体或其他方法）
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;

    unsigned int starVAO;
    unsigned int starVBO;
    unsigned int starEBO;
	unsigned int coronaQuadVAO;
	unsigned int coronaQuadVBO;
    unsigned int lensQuadVAO;
    unsigned int lensQuadVBO;
    unsigned int volQuadVAO;
    unsigned int volQuadVBO;

    struct LensFlare
    {
        unsigned int textureID;
		glm::vec4    tint;  // RGBA颜色 控制光晕的颜色和透明度
		float        sizeMult;  // 控制光晕的缩放大小
    };

    // 恒星位置
    glm::vec3 pointSunPositions = glm::vec3(-50.0f, 50.0f, -600.0f);
	// 恒星缩放
    glm::vec3 SunScale = glm::vec3(120.0f);
    // 更暖的颜色
    glm::vec3 starColor = glm::vec3(1.0f, 0.65f, 0.3f);
    // 更暖的核心颜色 
    glm::vec3 coreColor = glm::vec3(1.0f, 0.75f, 0.4f); 


    Sun();
    Sun(unsigned int& starVAO, unsigned int& starVBO, unsigned int& starEBO, 
        unsigned int& coronaQuadVAO, unsigned int& coronaQuadVBO);

    void BallGenerate();
	void CoreInit();
	void CoronaQuadInit();
    void LensQuadInit();
	void VolQuadInit();

    void SunRender(Shader& sunCoreShader, Shader& sunCoronaShader, Shader& CoreCoronaShader, Shader& sunGlowShader,
        const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view);

	void LensFlareRender(Shader& lensFlareShader, 
        const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view, 
        const std::vector<LensFlare>& flareTexture);

    void SunRenderPlus(Shader& sunCoreShader, Shader& sunCoronaShader, Shader& CoreCoronaShader, 
        Shader& sunGlowShader, Shader& sunVolShader,
        const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view);

    /*void CoreRender(Shader& sunCoreShader);
	void CoronaQuadRender(Shader& sunCoronaShader);
	void CoronaBallRender(Shader& CoreCoronaShader);
	void GlowRender(Shader& sunGlowShader);*/


    

    

private:
	/*Camera_ver2 camera;
    glm::mat4 projection;
	glm::mat4 view;*/
};

