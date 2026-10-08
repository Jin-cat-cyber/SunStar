#pragma once
#include <glm/glm.hpp>

//  时间抗锯齿（TAA）：抖动序列。纯函数与常量，状态在 Globals.h 的 gTaa。
//  推导与判据见 Tech_finding_Report/时间抗锯齿（TAA）_设计与实现计划书.md。
namespace taa
{
    inline constexpr int   JITTER_N = 8;     // 序列长度（计划书 §11）
    inline constexpr float JITTER_PX = 0.40f;  // 幅度 ±0.40 像素（式 24）。必须在 jitter() 里乘上去才生效，见文件末
    inline constexpr float W_CURRENT = 0.25f;  // 当前帧权重（式 12：N_eff = 4，半衰期 2.41 帧）。2026-10-08 由 0.1 调轻：0.1 时历史占九成，画面糊且面状沸腾
    inline constexpr float W_RESET = 1.0f;  // 失效帧：全取当前帧（§8.2）
    inline constexpr float W_WARP = 0.5f;  // 折跃活跃期（§8.3）
    inline constexpr float GAMMA = 1.5f;  // variance clipping 的宽度系数（式 15）。2026-10-08 由 1.0 放宽：低对比区标准差趋近 0 时区间塌掉，历史被整块夹掉
    inline constexpr float SHARPEN = 0.15f; // 写回时的反锐化强度（0 = 关）。2026-10-08 由 0.40 下调：它同时在放大残余的时间噪声

    //  基数 b 的 radical inverse：把 n 的 b 进制数字反转（式 6）。
    inline float radicalInverse(int n, int b)
    {
        float f = 1.0f;
        float r = 0.0f;
        while (n > 0)
        {
            f /= static_cast<float>(b);
            r += f * static_cast<float>(n % b);
            n /= b;
        }
        return r;
    }

    //  第 i 帧的像素偏移，已平移到以 0 为中心（式 7）。
    //  外部传 0 起算的下标，内部按 1 起算，好与计划书 §5.1 那张表逐项对上。
    inline glm::vec2 jitter(int i)
    {
        const int n = i + 1;
        //  JITTER_PX 必须在这里乘上去：radicalInverse 减 0.5 的值域是 [-0.5, 0.5)，乘 2*JITTER_PX 之后幅度才是 ±JITTER_PX 像素。
        //  2026-10-08 之前这里漏了这一步，JITTER_PX 是个死常量（改它没有任何反应）。
        return glm::vec2((radicalInverse(n, 2) - 0.5f) * (2.0f * JITTER_PX),
                         (radicalInverse(n, 3) - 0.5f) * (2.0f * JITTER_PX));
    }
}