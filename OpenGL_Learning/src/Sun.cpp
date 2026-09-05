#include "Sun.h"
#include "Globals.h"

Sun::Sun() :
	starVAO(0), starVBO(0), starEBO(0), coronaQuadVAO(0), coronaQuadVBO(0)
{
    BallGenerate();
}


Sun::Sun(unsigned int& StarVAO, unsigned int& StarVBO, unsigned int& StarEBO, 
        unsigned int& CoronaQuadVAO, unsigned int& CoronaQuadVBO) :
	starVAO(StarVAO), starVBO(StarVBO), starEBO(StarEBO), 
    coronaQuadVAO(CoronaQuadVAO), coronaQuadVBO(CoronaQuadVBO)
{
    BallGenerate();
	CoreInit();
    CoronaQuadInit();
    LensQuadInit();
    VolQuadInit();
}


void Sun::BallGenerate()
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


void Sun::CoreInit()
{
    glGenVertexArrays(1, &starVAO);
    glGenBuffers(1, &starVBO);
    glGenBuffers(1, &starEBO);
    glBindVertexArray(starVAO);
    glBindBuffer(GL_ARRAY_BUFFER, starVBO);
    glBufferData(GL_ARRAY_BUFFER, starVertices.size() * sizeof(float), &starVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, starIndices.size() * sizeof(unsigned int), &starIndices[0], GL_STATIC_DRAW);
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // 纹理坐标属性
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void Sun::CoronaQuadInit()
{
    float coronaQuadVertices[] = {
        // positions   // texCoords
        /*-1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f*/
         // positions        // texcoords
        -2.0f, -2.0f, 0.0f,  0.0f, 0.0f,
         2.0f, -2.0f, 0.0f,  1.0f, 0.0f,
         2.0f,  2.0f, 0.0f,  1.0f, 1.0f,
        -2.0f,  2.0f, 0.0f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &coronaQuadVAO);
    glGenBuffers(1, &coronaQuadVBO);
    glBindVertexArray(coronaQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, coronaQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coronaQuadVertices), coronaQuadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Sun::LensQuadInit()
{
    float lensQuadVertices[] = {
        // positions   // texCoords
        /*-1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f*/
         // positions        // texcoords
        -2.0f, -2.0f, 0.0f,  0.0f, 0.0f,
         2.0f, -2.0f, 0.0f,  1.0f, 0.0f,
         2.0f,  2.0f, 0.0f,  1.0f, 1.0f,
        -2.0f,  2.0f, 0.0f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &lensQuadVAO);
    glGenBuffers(1, &lensQuadVBO);
    glBindVertexArray(lensQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lensQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lensQuadVertices), lensQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Sun::VolQuadInit()
{
    float VolQuadVertices[] = {
        // 位置(x,y)      纹理坐标(u,v)
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f
    };

    glGenVertexArrays(1, &volQuadVAO);
    glGenBuffers(1, &volQuadVBO);
    glBindVertexArray(volQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, volQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VolQuadVertices), VolQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Sun::SunRender(Shader& sunCoreShader, Shader& sunCoronaShader, Shader& CoreCoronaShader, Shader& sunGlowShader,
    const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view)
{
  /*  GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
    GLboolean blendEnabled;
    glGetBooleanv(GL_BLEND, &blendEnabled);
    GLint depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);*/
    
    // ========================================
        // 恒星渲染部分 - 开始
        // ========================================
    float starTime = static_cast<float>(glfwGetTime()); // 恒星旋转时间
    float starPulse = 1.0f + sin(starTime * 1.5f) * 0.0003f; // 计算脉冲效果
    float starIntensity = 3.0f + sin(starTime * 0.7f) * 0.3f; // 计算恒星亮度变化

    // 重置深度状态，确保每层都独立
    glEnable(GL_DEPTH_TEST);

    // 1. 先绘制核心层（最小，最亮）
    glDepthMask(GL_TRUE); // 核心层深度写入
    glDisable(GL_BLEND); // 核心不透明，不需要混合
    glDepthFunc(GL_LESS); // 默认深度函数
    glDisable(GL_BLEND); // 核心不透明，不需要混合

    float starPulse2 = starPulse * 0.9f; // 核心脉冲幅度更小一些
    sunCoreShader.use();
    glm::mat4 coreModel = glm::mat4(1.0f);
    coreModel = glm::translate(coreModel, pointSunPositions);
    coreModel = glm::rotate(coreModel, starTime * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
    coreModel = glm::scale(coreModel, SunScale * 1.0f * starPulse2);

    // HDR增强核心亮度
    float starHDRMutilplier = 12.0f;

    sunCoreShader.setMat4("model", coreModel);
    sunCoreShader.setMat4("view", view);
    sunCoreShader.setMat4("projection", projection);
    sunCoreShader.setFloat("time", starTime);
    sunCoreShader.setVec3("starColor", starColor);
    //sunCoreShader.setVec3("coreColor", glm::vec3(1.0f, 0.9f, 0.7f));
    sunCoreShader.setVec3("coreColor", coreColor);
    sunCoreShader.setVec3("viewPos", camera.Position);
    sunCoreShader.setFloat("intensity", starIntensity * starHDRMutilplier);

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    //—————————————————————————————————————————————————————————————————————


    //—————————————————————————————————————————————————————————————————————
    // 2. 绘制日冕层（中等大小）
    //  关键：重置深度状态，使用不同的深度函数
    glDepthMask(GL_FALSE); //  日冕层不写入深度
    glDepthFunc(GL_LEQUAL); //  允许深度值相等或更大的片段通过
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合让日冕更亮

    // 喷涌日冕
    sunCoronaShader.use();
    glm::vec3 dirToCamera = camera.Position - pointSunPositions;
    glm::mat4 coronaModel = glm::mat4(1.0f);
    coronaModel = glm::translate(coronaModel, pointSunPositions);
    glm::mat4 rotate = glm::inverse(glm::lookAt(glm::vec3(0.f), dirToCamera, camera.WorldUp));
    coronaModel = coronaModel * rotate;

    coronaModel = glm::scale(coronaModel, SunScale * 2.0f * starPulse); // 日冕层比核心大

    sunCoronaShader.setMat4("model", coronaModel);
    sunCoronaShader.setMat4("view", view);
    sunCoronaShader.setMat4("projection", projection);
    sunCoronaShader.setFloat("time", starTime * 0.08f);
    sunCoronaShader.setVec3("sunCenter", pointSunPositions);
    sunCoronaShader.setVec3("coronaColor", starColor);
    //sunCoronaShader.setVec3("viewPos", camera.Position);
    sunCoronaShader.setFloat("coronaIntensity", 1.8f);
    sunCoronaShader.setFloat("scale", SunScale.x * 1.7f); // 204.0f

    glBindVertexArray(coronaQuadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   // 用4个顶点绘制四边形
    glBindVertexArray(0);

    // 环绕日冕
    CoreCoronaShader.use();
    coronaModel = glm::mat4(1.0f);
    coronaModel = glm::translate(coronaModel, pointSunPositions);
    coronaModel = glm::rotate(coronaModel, starTime * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
    coronaModel = glm::scale(coronaModel, SunScale * 1.5f * starPulse); // 日冕层比核心大

    CoreCoronaShader.setMat4("model", coronaModel);
    CoreCoronaShader.setMat4("view", view);
    CoreCoronaShader.setMat4("projection", projection);
    CoreCoronaShader.setFloat("time", starTime);
    CoreCoronaShader.setVec3("sunCenter", pointSunPositions);
    CoreCoronaShader.setVec3("coronaColor", starColor);
    //CoreCoronaShader.setVec3("viewPos", camera.Position);
    CoreCoronaShader.setFloat("coronaIntensity", 1.8f);
    CoreCoronaShader.setFloat("coronaRadius", SunScale.x * 1.7f); // 204.0f

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    //—————————————————————————————————————————————————————————————————————
    // 3. 最后绘制辉光层（最大，最透明）
    // 修改深度状态，混合状态
    glDepthMask(GL_FALSE); // 不写入深度
    glDepthFunc(GL_LEQUAL); // 等于或更大深度通过
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合！

    sunGlowShader.use();
    glm::mat4 glowModel = glm::mat4(1.0f);
    glowModel = glm::translate(glowModel, pointSunPositions);
    glowModel = glm::rotate(glowModel, starTime * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
    glowModel = glm::scale(glowModel, SunScale * 1.5f * starPulse);

    sunGlowShader.setMat4("model", glowModel);
    sunGlowShader.setMat4("view", view);
    sunGlowShader.setMat4("projection", projection);
    sunGlowShader.setFloat("time", starTime);
    sunGlowShader.setVec3("starColor", starColor);
    sunGlowShader.setVec3("viewPos", camera.Position);
    sunGlowShader.setFloat("glowIntensity", 0.8f);

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    //—————————————————————————————————————————————————————————————————————

    // ========================================
    // 恒星渲染部分 - 结束
    // ========================================

    //// 恢复原始状态
    //if (!depthEnabled) glDisable(GL_DEPTH_TEST);
    //else glEnable(GL_DEPTH_TEST);
    //glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
    //glDepthFunc(depthFunc);
    //if (blendEnabled) glEnable(GL_BLEND);
    //else glDisable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数

}


void Sun::LensFlareRender(Shader& lensFlareShader, 
    const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view, const std::vector<LensFlare>& flareTexture)
{
    glDepthMask(GL_FALSE); // 不写入深度
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合,保留Alpha控制强度
    
    lensFlareShader.use();
    lensFlareShader.setMat4("view", view);
    lensFlareShader.setMat4("projection", projection);

    glm::vec3 dirToCamera2 = camera.Position - pointSunPositions;
    glm::mat4 flareRotate = glm::inverse(glm::lookAt(glm::vec3(0.f), dirToCamera2, camera.WorldUp));

    for (const auto& flare : flareTexture)
    {
        glm::mat4 flareModel = glm::mat4(1.0f);
        flareModel = glm::translate(flareModel, pointSunPositions);
        // 公告板始终面向相机
        flareModel = flareModel * flareRotate;
        // 光晕大小，比辉光略大
        float flareSize = SunScale.x * flare.sizeMult;
        flareModel = glm::scale(flareModel, glm::vec3(flareSize));
        
        lensFlareShader.setMat4("model", flareModel); 
        lensFlareShader.setVec4("flareColor", flare.tint); // 光晕颜色和透明度
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flare.textureID);
        lensFlareShader.setInt("flareTexture", 0);
        
        glBindVertexArray(lensQuadVAO); // 复用光晕四边形
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);

     // 恢复深度测试和混合状态
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数
}

void Sun::SunRenderPlus(Shader& sunCoreShader, Shader& sunCoronaShader, Shader& CoreCoronaShader, Shader& sunGlowShader, Shader& sunVolShader,
    const Camera_ver2& camera, const glm::mat4& projection, const glm::mat4& view)
{
    /*  GLboolean depthEnabled;
      glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
      GLboolean blendEnabled;
      glGetBooleanv(GL_BLEND, &blendEnabled);
      GLint depthFunc;
      glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
      GLboolean depthMask;
      glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);*/

      // ========================================
          // 恒星渲染部分 - 开始
          // ========================================
    float starTime = static_cast<float>(glfwGetTime()); // 恒星旋转时间
    float starPulse = 1.0f + sin(starTime * 1.5f) * 0.0003f; // 计算脉冲效果
    float starIntensity = 3.0f + sin(starTime * 0.7f) * 0.3f; // 计算恒星亮度变化

    // 重置深度状态，确保每层都独立
    glEnable(GL_DEPTH_TEST);

    // 1. 先绘制核心层（最小，最亮）
    glDepthMask(GL_TRUE); // 核心层深度写入
    glDisable(GL_BLEND); // 核心不透明，不需要混合
    glDepthFunc(GL_LESS); // 默认深度函数
    glDisable(GL_BLEND); // 核心不透明，不需要混合

    float starPulse2 = starPulse * 0.9f; // 核心脉冲幅度更小一些
    sunCoreShader.use();
    glm::mat4 coreModel = glm::mat4(1.0f);
    coreModel = glm::translate(coreModel, pointSunPositions);
    coreModel = glm::rotate(coreModel, starTime * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
    coreModel = glm::scale(coreModel, SunScale * 1.0f * starPulse2);

    // HDR增强核心亮度
    float starHDRMutilplier = 12.0f;

    sunCoreShader.setMat4("model", coreModel);
    sunCoreShader.setMat4("view", view);
    sunCoreShader.setMat4("projection", projection);
    sunCoreShader.setFloat("time", starTime);
    sunCoreShader.setVec3("starColor", starColor);
    //sunCoreShader.setVec3("coreColor", glm::vec3(1.0f, 0.9f, 0.7f));
    sunCoreShader.setVec3("coreColor", coreColor);
    sunCoreShader.setVec3("viewPos", camera.Position);
    sunCoreShader.setFloat("intensity", starIntensity * starHDRMutilplier);

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    //—————————————————————————————————————————————————————————————————————


    //—————————————————————————————————————————————————————————————————————
    // 2. 绘制日冕层（中等大小）
    //  关键：重置深度状态，使用不同的深度函数
    glDepthMask(GL_FALSE); //  日冕层不写入深度
    glDepthFunc(GL_LEQUAL); //  允许深度值相等或更大的片段通过
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合让日冕更亮

    // 喷涌日冕
    sunCoronaShader.use();
    glm::vec3 dirToCamera = camera.Position - pointSunPositions;
    glm::mat4 coronaModel = glm::mat4(1.0f);
    coronaModel = glm::translate(coronaModel, pointSunPositions);
    glm::mat4 rotate = glm::inverse(glm::lookAt(glm::vec3(0.f), dirToCamera, camera.WorldUp));
    coronaModel = coronaModel * rotate;

    coronaModel = glm::scale(coronaModel, SunScale * 2.0f * starPulse); // 日冕层比核心大

    sunCoronaShader.setMat4("model", coronaModel);
    sunCoronaShader.setMat4("view", view);
    sunCoronaShader.setMat4("projection", projection);
    sunCoronaShader.setFloat("time", starTime * 0.08f);
    sunCoronaShader.setVec3("sunCenter", pointSunPositions);
    sunCoronaShader.setVec3("coronaColor", starColor);
    //sunCoronaShader.setVec3("viewPos", camera.Position);
    sunCoronaShader.setFloat("coronaIntensity", 1.8f);
    sunCoronaShader.setFloat("scale", SunScale.x * 1.7f); // 204.0f

    glBindVertexArray(coronaQuadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   // 用4个顶点绘制四边形
    glBindVertexArray(0);

    // 环绕日冕
    CoreCoronaShader.use();
    coronaModel = glm::mat4(1.0f);
    coronaModel = glm::translate(coronaModel, pointSunPositions);
    coronaModel = glm::rotate(coronaModel, starTime * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
    coronaModel = glm::scale(coronaModel, SunScale * 1.5f * starPulse); // 日冕层比核心大

    CoreCoronaShader.setMat4("model", coronaModel);
    CoreCoronaShader.setMat4("view", view);
    CoreCoronaShader.setMat4("projection", projection);
    CoreCoronaShader.setFloat("time", starTime);
    CoreCoronaShader.setVec3("sunCenter", pointSunPositions);
    CoreCoronaShader.setVec3("coronaColor", starColor);
    //CoreCoronaShader.setVec3("viewPos", camera.Position);
    CoreCoronaShader.setFloat("coronaIntensity", 1.8f);
    CoreCoronaShader.setFloat("coronaRadius", SunScale.x * 1.7f); // 204.0f

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    //—————————————————————————————————————————————————————————————————————
    // 3. 最后绘制辉光层（最大，最透明）
    // 修改深度状态，混合状态
    glDepthMask(GL_FALSE); // 不写入深度
    glDepthFunc(GL_LEQUAL); // 等于或更大深度通过
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合！

    sunGlowShader.use();
    glm::mat4 glowModel = glm::mat4(1.0f);
    glowModel = glm::translate(glowModel, pointSunPositions);
    glowModel = glm::rotate(glowModel, starTime * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
    glowModel = glm::scale(glowModel, SunScale * 1.5f * starPulse);

    sunGlowShader.setMat4("model", glowModel);
    sunGlowShader.setMat4("view", view);
    sunGlowShader.setMat4("projection", projection);
    sunGlowShader.setFloat("time", starTime);
    sunGlowShader.setVec3("starColor", starColor);
    sunGlowShader.setVec3("viewPos", camera.Position);
    sunGlowShader.setFloat("glowIntensity", 0.8f);

    glBindVertexArray(starVAO);
    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    // ========================================
    // 4. 恒星体积光（包围恒星的体积球，纯光雾无阴影）
    // ========================================
    // 当前状态已满足：glDepthMask(GL_FALSE)、加法混合、GL_LEQUAL
    sunVolShader.use();
    sunVolShader.setVec3("camPos", camera.Position);
    sunVolShader.setVec3("sunCenter", pointSunPositions);
    sunVolShader.setFloat("sunRadius", SunScale.x * 5.0f);   // 可调接口：改这个 3.0 系数
    sunVolShader.setFloat("time", starTime);
    sunVolShader.setVec3("sunColor", starColor);
    sunVolShader.setVec2("resolution", glm::vec2((float)windowwidth, (float)windowheight));
    sunVolShader.setMat4("invProjView", glm::inverse(projection* view));

    glBindVertexArray(volQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    // ========================================
    //—————————————————————————————————————————————————————————————————————

    // ========================================
    // 恒星渲染部分 - 结束
    // ========================================

    //// 恢复原始状态
    //if (!depthEnabled) glDisable(GL_DEPTH_TEST);
    //else glEnable(GL_DEPTH_TEST);
    //glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
    //glDepthFunc(depthFunc);
    //if (blendEnabled) glEnable(GL_BLEND);
    //else glDisable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数

}


//// 核心层
//void Sun::CoreRender(Shader& sunCoreShader)
//{
//
//    //float starPulse2 = starPulse * 0.9f; // 核心脉冲幅度更小一些
//    sunCoreShader.use();
//    glm::mat4 coreModel = glm::mat4(1.0f);
//    coreModel = glm::translate(coreModel, pointSunPositions);
//    coreModel = glm::rotate(coreModel, starTime * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
//    coreModel = glm::scale(coreModel, SunScale * 1.0f * starPulse2);
//
//    // HDR增强核心亮度
//    //float starHDRMutilplier = 12.0f;
//
//    sunCoreShader.setMat4("model", coreModel);
//    sunCoreShader.setMat4("view", view);
//    sunCoreShader.setMat4("projection", projection);
//    sunCoreShader.setFloat("time", starTime);
//    sunCoreShader.setVec3("starColor", starColor);
//    //sunCoreShader.setVec3("coreColor", glm::vec3(1.0f, 0.9f, 0.7f));
//    sunCoreShader.setVec3("coreColor", coreColor);
//    sunCoreShader.setVec3("viewPos", camera.Position);
//    sunCoreShader.setFloat("intensity", starIntensity * starHDRMutilplier);
//
//    glBindVertexArray(starVAO);
//    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//}
//
//// 喷涌日冕
//void Sun::CoronaQuadRender(Shader& sunCoronaShader)
//{
//    
//    sunCoronaShader.use();
//    glm::vec3 dirToCamera = camera.Position - pointSunPositions;
//    glm::mat4 coronaModel = glm::mat4(1.0f);
//    coronaModel = glm::translate(coronaModel, pointSunPositions);
//    glm::mat4 rotate = glm::inverse(glm::lookAt(glm::vec3(0.f), dirToCamera, camera.WorldUp));
//    coronaModel = coronaModel * rotate;
//
//    coronaModel = glm::scale(coronaModel, SunScale * 2.0f * starPulse); // 日冕层比核心大
//
//    sunCoronaShader.setMat4("model", coronaModel);
//    sunCoronaShader.setMat4("view", view);
//    sunCoronaShader.setMat4("projection", projection);
//    sunCoronaShader.setFloat("time", starTime * 0.08f);
//    sunCoronaShader.setVec3("sunCenter", pointSunPositions);
//    sunCoronaShader.setVec3("coronaColor", starColor);
//    //sunCoronaShader.setVec3("viewPos", camera.Position);
//    sunCoronaShader.setFloat("coronaIntensity", 1.8f);
//    sunCoronaShader.setFloat("scale", SunScale.x * 1.7f); // 204.0f
//
//    glBindVertexArray(coronaQuadVAO);
//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   // 用4个顶点绘制四边形
//    glBindVertexArray(0);
//}
//
//// 环绕日冕
//void Sun::CoronaBallRender(Shader& CoreCoronaShader)
//{
//    
//    CoreCoronaShader.use();
//    glm::vec3 dirToCamera = camera.Position - pointSunPositions;
//    glm::mat4 coronaModel = glm::mat4(1.0f);
//    coronaModel = glm::translate(coronaModel, pointSunPositions);
//    coronaModel = glm::rotate(coronaModel, starTime * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
//    coronaModel = glm::scale(coronaModel, SunScale * 1.6f * starPulse); // 日冕层比核心大
//
//    CoreCoronaShader.setMat4("model", coronaModel);
//    CoreCoronaShader.setMat4("view", view);
//    CoreCoronaShader.setMat4("projection", projection);
//    CoreCoronaShader.setFloat("time", starTime);
//    CoreCoronaShader.setVec3("sunCenter", pointSunPositions);
//    CoreCoronaShader.setVec3("coronaColor", starColor);
//    //CoreCoronaShader.setVec3("viewPos", camera.Position);
//    CoreCoronaShader.setFloat("coronaIntensity", 1.8f);
//    CoreCoronaShader.setFloat("coronaRadius", SunScale.x * 1.7f); // 204.0f
//
//    glBindVertexArray(starVAO);
//    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//}
//
//// 辉光
//void Sun::GlowRender(Shader& sunGlowShader)
//{
//    glDepthMask(GL_FALSE); // 不写入深度
//    glDepthFunc(GL_LEQUAL); // 等于或更大深度通过
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合！
//
//    sunGlowShader.use();
//    glm::mat4 glowModel = glm::mat4(1.0f);
//    glowModel = glm::translate(glowModel, pointSunPositions);
//    glowModel = glm::rotate(glowModel, starTime * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
//    glowModel = glm::scale(glowModel, SunScale * 1.6f * starPulse);
//
//    sunGlowShader.setMat4("model", glowModel);
//    sunGlowShader.setMat4("view", view);
//    sunGlowShader.setMat4("projection", projection);
//    sunGlowShader.setFloat("time", starTime);
//    sunGlowShader.setVec3("starColor", starColor);
//    sunGlowShader.setVec3("viewPos", camera.Position);
//    sunGlowShader.setFloat("glowIntensity", 0.8f);
//
//    glBindVertexArray(starVAO);
//    glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//}
