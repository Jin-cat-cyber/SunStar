#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>



void BallGenerate(std::vector<float>& starVertices, std::vector<unsigned int>& starIndices,
    const unsigned int X_SEGMENTS, const unsigned int Y_SEGMENTS, const float PI);

unsigned int loadTexture(char const* path);     // 纹理加载函数
unsigned int loadCubemap(std::vector<std::string> faces);   // 立方体贴图加载函数

void renderCube();
void renderQuad();
