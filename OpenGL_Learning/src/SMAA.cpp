#include <glad/glad.h>

#include "Globals.h"
#include "SMAA.h"

#include "Shader.h"

#include <glm/glm.hpp>

//  SMAA 的实现在这里；资源（smaaFBO / smaaColorBuffer / smaaEdgeFBO / smaaEdgeTex）与三个观感
//  常量都在 Globals.h。本模块只做"建资源 / 删资源 / 两遍 + 两次 blit"，不缓存任何帧内量。

void smaa::Init(int width, int height)
{
    smaa::Release();   // 重复调用安全：先删再建

    //  颜色两张都用 RGBA8 / LINEAR：读出来的是 [0,1] 的 LDR 图，混邻居必须 LINEAR。
    glGenTextures(2, smaaColorBuffer);
    glGenFramebuffers(2, smaaFBO);
    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, smaaColorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindFramebuffer(GL_FRAMEBUFFER, smaaFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smaaColorBuffer[i], 0);
    }

    //  边缘图：RG8 + NEAREST。第二遍是逐纹素读标记，绝不能线性过滤 —— 插值出来的小数标记
    //  会让"这条边到哪儿结束"的判断整个失真。
    glGenTextures(1, &smaaEdgeTex);
    glBindTexture(GL_TEXTURE_2D, smaaEdgeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &smaaEdgeFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, smaaEdgeFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smaaEdgeTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void smaa::Release()
{
    for (int i = 0; i < 2; ++i)
    {
        if (smaaFBO[i]) { glDeleteFramebuffers(1, &smaaFBO[i]); smaaFBO[i] = 0; }
        if (smaaColorBuffer[i]) { glDeleteTextures(1, &smaaColorBuffer[i]); smaaColorBuffer[i] = 0; }
    }
    if (smaaEdgeFBO) { glDeleteFramebuffers(1, &smaaEdgeFBO); smaaEdgeFBO = 0; }
    if (smaaEdgeTex) { glDeleteTextures(1, &smaaEdgeTex); smaaEdgeTex = 0; }
}
void smaa::Pass(Shader& edgeShader, Shader& blendShader, int width, int height, unsigned int quadVAO)
{
    //  GL 状态按【真值】保存再还原（工程里有过"以为在还原、其实是首次开启"的坑）。
    const GLboolean depthWas = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendWas = glIsEnabled(GL_BLEND);
    GLboolean depthMaskWas = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskWas);

    const float texelX = 1.0f / static_cast<float>(width);
    const float texelY = 1.0f / static_cast<float>(height);

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    //  (1) 默认帧缓冲上的合成结果 -> smaaColorBuffer[0]（同尺寸，直拷）
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, smaaFBO[0]);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    //  (2) 边缘检测：smaaColorBuffer[0] -> smaaEdgeTex
    edgeShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, smaaColorBuffer[0]);
    edgeShader.setInt("uColor", 0);
    edgeShader.setVec2("uTexel", glm::vec2(texelX, texelY));
    edgeShader.setFloat("uThreshold", SMAA_THRESHOLD);
    edgeShader.setFloat("uLocalContrast", SMAA_LOCAL_CONTRAST);
    glBindFramebuffer(GL_FRAMEBUFFER, smaaEdgeFBO);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //  (3) 邻域混合：smaaColorBuffer[0] + smaaEdgeTex -> smaaColorBuffer[1]
    blendShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, smaaColorBuffer[0]);
    blendShader.setInt("uColor", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, smaaEdgeTex);
    blendShader.setInt("uEdges", 1);
    blendShader.setVec2("uTexel", glm::vec2(texelX, texelY));
    blendShader.setInt("uMaxSteps", SMAA_MAX_STEPS);
    glBindFramebuffer(GL_FRAMEBUFFER, smaaFBO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //  (4) 拷回默认帧缓冲
    glBindFramebuffer(GL_READ_FRAMEBUFFER, smaaFBO[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    //  还原 GL 状态（用存下来的真值）
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (depthWas) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (blendWas) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    glDepthMask(depthMaskWas);
}