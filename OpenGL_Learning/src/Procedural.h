#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>


// ===== 视锥体剔除 =====
struct FrustumPlanes { glm::vec4 p[6]; };       // xyz=法线，w=距离

FrustumPlanes ExtractFrustum(const glm::mat4& m);
bool SphereInFrustum(const FrustumPlanes& fp, const glm::vec3& c, float r);
bool CanCastVisibleShadow(const FrustumPlanes& fp, const glm::vec3& lightPos, const glm::vec3& p);

void BallGenerate(std::vector<float>& starVertices, std::vector<unsigned int>& starIndices,
    const unsigned int X_SEGMENTS, const unsigned int Y_SEGMENTS, const float PI);

unsigned int loadTexture(char const* path);     // 纹理加载函数
unsigned int loadCubemap(std::vector<std::string> faces);   // 立方体贴图加载函数

void renderCube();
void renderQuad();
