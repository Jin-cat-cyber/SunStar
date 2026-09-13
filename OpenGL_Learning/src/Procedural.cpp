#include "Procedural.h"
#include <cmath>
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


FrustumPlanes ExtractFrustum(const glm::mat4& m)
{
    // glm 列主序：m[col][row]，逐行取出
    glm::vec4 r0(m[0][0], m[1][0], m[2][0], m[3][0]);
    glm::vec4 r1(m[0][1], m[1][1], m[2][1], m[3][1]);
    glm::vec4 r2(m[0][2], m[1][2], m[2][2], m[3][2]);
    glm::vec4 r3(m[0][3], m[1][3], m[2][3], m[3][3]);

    glm::vec4 pl[6] = { r3 + r0, r3 - r0,   // left, right
                        r3 + r1, r3 - r1,   // bottom, top
                        r3 + r2, r3 - r2 }; // near, far

    FrustumPlanes f;
    for (int i = 0; i < 6; ++i)
    {
        // 关键：只归一化法线部分，让 w 变成"垂直距离"。
        // 不能用 glm::normalize(vec4)（会除以四维模长，导致球测试的 +r 量纲错）。
        glm::vec3 n(pl[i].x, pl[i].y, pl[i].z);
        float invLen = 1.0f / glm::length(n);
        f.p[i] = glm::vec4(n * invLen, pl[i].w * invLen);
    }
    return f;
}

// ===== 包围球视锥测试：球心到各平面距离 + 半径 < 0 → 在平面外 → 剔除 =====
bool SphereInFrustum(const FrustumPlanes& fp, const glm::vec3& c, float r)
{
    for (int i = 0; i < 6; i++)
    {
        if (glm::dot(glm::vec3(fp.p[i]), c) + fp.p[i].w + r < 0.0f)
            return false;
    }
    return true;
}

// 判定：小行星 p 能否投影到"相机看得见的表面"上
// 判据 = 从光源穿过 p 的射线，是否在"越过 p 之后"进入相机视锥（即可见面在 p 后方 → p 能挡到它）
bool CanCastVisibleShadow(const FrustumPlanes& fp, const glm::vec3& lightPos, const glm::vec3& p)
{
    glm::vec3 dir = glm::normalize(p - lightPos);
    float Lp = glm::length(p - lightPos);

    float tEnter = -1e30f, tExit = 1e30f;
    for (int i = 0; i < 6; ++i)
    {
        const glm::vec4& pl = fp.p[i];
        glm::vec3 n(pl.x, pl.y, pl.z);
        // 内部点满足 dot(n,x)+w >= 0；x(t)=lightPos+t*dir
        // => t*dot(n,dir) >= -w - dot(n, lightPos)
        float denom = glm::dot(n, dir);
        float num   = -pl.w - glm::dot(n, lightPos);
        if (std::abs(denom) < 1e-6f)
        {
            if (num > 0.0f) return false;   // 平行且起点在外侧 → 整条光线在外
        }
        else
        {
            float t = num / denom;
            if (denom > 0.0f) tEnter = std::max(tEnter, t);
            else              tExit  = std::min(tExit,  t);
        }
    }
    if (tEnter > tExit) return false;       // 射线没穿过视锥
    return Lp < tExit;                      // 进入点在越过 p 之后 → p 能投影到可见面
}

// 把包围球(center, radius)分配到它覆盖的立方体面；返回面数，faces[] 写索引
int AssignCasterFaces(const glm::vec3& lightPos, const glm::vec3& center,
    float radius, int faces[6])
{
    static const glm::vec3 AXIS[6] = {
        { 1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        { 0.0f, 1.0f, 0.0f}, { 0.0f,-1.0f, 0.0f},
        { 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f,-1.0f}
    };
    glm::vec3 d = center - lightPos;
    float L = glm::length(d);
    if (L < 1e-6f) 
    {
        faces[0] = 0;
        return 1;
    }
    glm::vec3 dir = d / L;

    // 球张开的角半径
    float alphaDeg = glm::degrees(std::asin(glm::clamp(radius / L, 0.0f, 1.0f)));
    // 45° + 角半径 + 1.5° 边界余量
    float cosLimit = std::cos(glm::radians(45.0f + alphaDeg + 1.5f));

    int n = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (glm::dot(dir, AXIS[i]) >= cosLimit)
            faces[n++] = i;
    }
    return n;
}

// ===== 屏幕占比函数 =====
float ScreenSizePx(float worldRadius, float dist, float viewportH, float fovRad)
{
    return (worldRadius / dist) * (viewportH / (2.0f * std::tan(fovRad * 0.5f)));
}


// 加载纹理函数
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 加载立方体贴图
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Ensure cubemap faces are not flipped vertically
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    return textureID;
}

// 球体生成程序
void BallGenerate(std::vector<float>& starVertices, std::vector<unsigned int>& starIndices,
    const unsigned int X_SEGMENTS, const unsigned int Y_SEGMENTS, const float PI)
{
    // 生成球体顶点数据
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
            float yPos = cos(ySegment * PI);
            float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
            // 位置
            starVertices.push_back(xPos);
            starVertices.push_back(yPos);
            starVertices.push_back(zPos);
            // 法线
            starVertices.push_back(xPos);
            starVertices.push_back(yPos);
            starVertices.push_back(zPos);
            // 纹理坐标
            starVertices.push_back(xSegment);
            starVertices.push_back(ySegment);
        }
    }

    // 索引生成
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x)
        {
            starIndices.push_back(y * (X_SEGMENTS + 1) + x);
            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            starIndices.push_back(y * (X_SEGMENTS + 1) + x);
            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            starIndices.push_back(y * (X_SEGMENTS + 1) + x + 1);
        }
    }
}

// renderQuad() 在 NDC 中渲染一个 1x1 的 XY 四边形。
// ---------------------------------------------
void renderQuad()
{
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO = 0;

    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // 设置 plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


// renderCube() 渲染一个位于 NDC 中的 1x1 3D 立方体。
// -----------------------------------------
void renderCube()
{
    static unsigned int cubeVAO = 0;
    static unsigned int cubeVBO = 0;

    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // 填充缓冲区
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // 链接顶点属性
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}



