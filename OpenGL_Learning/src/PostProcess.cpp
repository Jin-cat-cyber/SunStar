#include "PostProcess.h"

void PostProcessing(Shader& brightPassShader, Shader& blurShader, Shader& compositeShader, const unsigned int& quadVAO)
{
    //==========================================
// 渲染到屏幕
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // 1. 亮度提取：从hdrColorBuffer中提取亮度信息到blurFBO1
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT); // 这个为什么只清理颜色缓冲?
    brightPassShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    brightPassShader.setInt("hdrImage", 0);
    brightPassShader.setFloat("threshold", 1.2f); // 设置亮度阈值 1.2f
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 2. 乒乓高斯模糊
    int blurPasses = 10;         // 6~10 越大越柔和
    bool horizontal = true;
    blurShader.use();
    for (int i = 0; i < blurPasses; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        blurShader.setBool("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        blurShader.setInt("image", 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //// MSAA帧缓冲到HDR帧缓冲的blit操作
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
    //glBlitFramebuffer(0, 0, windowwidth, windowheight,
    //    0, 0, windowwidth, windowheight,
    //    GL_COLOR_BUFFER_BIT, GL_NEAREST);

    //// 【新增】把深度从 msFBO 复制到默认帧缓冲，供后面的光晕使用
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    //glBlitFramebuffer(0, 0, windowwidth, windowheight,
    //    0, 0, windowwidth, windowheight,
    //    GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    //// 恢复为默认帧缓冲或绑定 hdrFBO 以便后续读取 hdrColorBuffer
    //glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // ------- 合成到屏幕 -------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 回到默认帧缓冲
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 老版本
    // 只清理颜色缓冲，不清理深度缓冲，以此来保留深度信息，确保后续渲染的物体不会被清除
    glClear(GL_COLOR_BUFFER_BIT);
    compositeShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    compositeShader.setInt("sceneTexture", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); // 最终模糊结果在 blurFBO1 的颜色附件
    compositeShader.setInt("bloomTexture", 1);
    compositeShader.setFloat("exposure", 1.0f); // 曝光值
    compositeShader.setFloat("bloomStrength", 0.6f); // Bloom强度
    compositeShader.setVec3("colorTint", glm::vec3(1.0f, 0.95f, 0.6f)); // Bloom颜色
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // blit hdrFBO depth to default FB for lens flare
    glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowwidth, windowheight,
        0, 0, windowwidth, windowheight,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

}
