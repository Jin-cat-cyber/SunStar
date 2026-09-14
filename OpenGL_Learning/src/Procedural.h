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
bool CanCastVisibleShadow(const FrustumPlanes& fp, const glm::vec3& lightPos, const glm::vec3& p, float r);

int AssignCasterFaces(const glm::vec3& lightPos, const glm::vec3& center, float radius, int faces[6]);

// ===== 屏幕占比函数 =====
float ScreenSizePx(float worldRadius, float dist, float viewportH, float fovRad);

// ===== 球体生成 =====
void BallGenerate(std::vector<float>& starVertices, std::vector<unsigned int>& starIndices,
    const unsigned int X_SEGMENTS, const unsigned int Y_SEGMENTS, const float PI);

// ===== 纹理加载 =====
unsigned int loadTexture(char const* path);     // 纹理加载函数
// ===== 立方体贴图加载 =====
unsigned int loadCubemap(std::vector<std::string> faces);   // 立方体贴图加载函数
// ==== 渲染立方体和四边形 =====
void renderCube();
void renderQuad();
