#pragma once
#include "Globals.h"
#include "Shader.h"

//  SMAA（形态学抗锯齿）：两遍全屏 + 两次 blit，作用在【合成之后的 LDR 图】上。
//  为什么吃 LDR：阈值是按亮度定的绝对量（0.06 = 亮度差 6%），tonemap + gamma 之后的图才是感知
//  均匀的 [0,1]；混合也是感知混合（把 10.0 的高光混进 0.05 的暗部会读成"边缘发光"）。
//  为什么用 blit 进出默认帧缓冲：这样不必改共享的 PostProcess.{h,cpp}，合成照旧写默认帧缓冲。
//  资源（4 张）与三个观感常量都在 Globals.h 里 inline 声明；开关是 M 键（smaaEnabled）。
//  调用顺序：setupFramebuffers 里 smaa::Init、每帧 PostProcessing 之后 smaa::Pass、
//  rebuildFramebuffers 里 smaa::Release。
namespace smaa
{
    //  建资源；可重复调用（内部先 Release 再建）。窗口尺寸变化时由 rebuildFramebuffers 调。
    void Init(int width, int height);

    //  删资源（窗口尺寸变化前，或退出前）
    void Release();

    //  执行两遍 + 两次 blit：读默认帧缓冲上的 LDR 合成结果，写回默认帧缓冲。
    //  必须在 PostProcessing(...) 之后、镜头光晕之前调用（光晕是加法叠加的成品，不该被磨边）。
    //  这里不判 smaaEnabled：开关留在调用点，一眼能看到它。
    void Pass(Shader& edgeShader, Shader& blendShader, int width, int height, unsigned int quadVAO);
}