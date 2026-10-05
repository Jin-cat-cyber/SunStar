#pragma once
// ---------------------------------------------------------------------------
//  WarpShock.h —— SC 分支折跃的冲击波（两道）：纯运动学，不含 GL。
//
//  两道波都是【平的圆环】：环躺在一张垂直于舰体纵轴的平面里，圆心落在那条纵轴上。
//  环的半径沿半径方向【匀速】推进：
//    去程收束波：从 S_OUT_R0 收到 0，与"舰体被抽走"的全程同长，在 τ* 那一刻正好收到圆心；
//    抵达发散波：从 0 散到 S_IN_R1，比线框淡完早 0.10 秒收掉。
//
//  【形状来由】改过两次：圆柱壳（侧视是一道竖直的墙）→ 球壳（像气泡）→ 平的圆环。
//  最终以参考画面为准：那是水波式的【同心圆】，而它的中心落在纵轴上。
//
//  ⚠️ 平的环没有厚度，所以【正侧看时会退化成一条线】—— 这是几何的固有代价，不是 bug。
//     片元里用 facing 对"侧对"做了补偿，档位见 S_FACING_FLOOR。
//
//  强度 = 能量律 × 窗口包络：
//    能量律 I(r) = (S_REF_R / r)^P —— 平面圆环是【二维】圆波，能量摊在周长 2πr 上，
//                 所以 P = 1（球面才是 2）。见 S_FALL_POW。
//    窗口包络 shockWindowOut / shockWindowIn 两端精确归零 —— 没有它，强度会在窗口边界
//                 被硬切，而能量律在两端都不是 0，于是最亮的一刻被一刀砍掉（实测踩过）。
//
//  与计划书的关系：机制与推导写在
//  `Tech_finding_Report/星际争霸2折跃_冲击波（收束与发散）_设计与实现计划书.md`（待写）。
//  【2026-10-03】该计划书已完成（14 节，含能量律、网格系数与行进色的推导），上面那句"待写"作废。
// ---------------------------------------------------------------------------
//  【现行】dClamp01 / dSmooth 的【定义】在 WarpDebris.h（第 87、88 行），这里是经下面这个头
//  转递进来的 —— WarpPillar.h 第 22 行自己就 include 了 WarpDebris.h（注释写着"复用
//  dRand / dLerp / dClamp01 / dSmooth"）。所以下面那句的传递链没错，只是没交代定义在哪一层，
//  留档对比：
#include "WarpPillar.h"     // 为了 P_ARRIVE_T0（抵达波从光柱出生面开始），顺带拿到 dClamp01

namespace warp_sc
{
    // ===== 去程收束波 ======================================================
    //  窗口 = 【扫描开始 → τ*】，也就是"舰体被抽走"的【全程】（0.60 秒）。
    //  ⚠️ 船不是蓄能完了才开始消失的：扫描从 FRONT_GEO_START = 0.95 就开始了，
    //     蓄能段 [0, 1.35] 与扫描段 [0.95, 1.55] 是【重叠】的。
    //     只取 [1.35, 1.55]（= T_VANISH，0.20 秒）的话，60 单位半径对应 300 单位/秒，
    //     30 fps 下只有 6 帧，读出来是"闪一下"而不是"一道波"。
    inline constexpr float S_OUT_T0 = FRONT_GEO_START;      // 0.95
    inline constexpr float S_OUT_T1 = TAU_STAR;             // 1.55：正好收束到轴心
    //inline constexpr float S_OUT_R0 = 60.0f;                // 起始半径；60 / 0.60 = 100 单位/秒
    inline constexpr float S_OUT_R0 = 50.0f;                // 起始半径；50 / 0.60 ≈ 83.3 单位/秒

    // ===== 抵达发散波 ======================================================
    //  起点 = 光柱出生的那一刻（同一张逻辑面），终点比线框淡完早 0.10 秒。
    inline constexpr float S_IN_T0 = P_ARRIVE_T0;           // 4.89
    inline constexpr float S_IN_T1 = T_TOTAL - 0.10f;       // 5.52
    inline constexpr float S_IN_R1 = 70.0f;                 // 终止半径；70 / 0.63 ≈ 111 单位/秒

    // ===== 通用 ============================================================
    // inline constexpr float S_HALF_LEN_K = 1.7f;  // 【已废】圆柱壳的轴向半长，球壳也不需要
    inline constexpr float S_REF_R = 60.0f;       // 能量律的参考半径：I(r) = (S_REF_R / r)^P
    inline constexpr float S_FALL_POW = 1.0f;     // 衰减指数 P：1.0 = 二维圆波（平面环的正确值）
                                                  //   2.0 是三维球面波（陡得多）；0.6 是"挂着不散"的人造观感
    inline constexpr float S_ENERGY_LO = 0.15f;   // 下限：扩散端别彻底看不见
    inline constexpr float S_ENERGY_HI = 8.0f;    // 上限：聚合端不许无限亮

    //  扭曲：单峰剖面 + 屏幕空间径向偏移。
    inline constexpr float S_PROFILE_W = 8.0f;  //12.0f;   // 剖面半宽（世界单位，沿半径方向）
    inline constexpr float S_DISTORT_PX = 12.0f;    //8.0f;   // 偏移幅度（像素，@ S_REF_H）
    inline constexpr float S_REF_H = 1080.0f;     // 上面那个像素数对应的屏幕高度

    // ===== 环带的可见宽度与"侧对补偿" =====================================
    //  环带的世界半宽。环【不是】靠网格做出来的，而是片元按半径把带外丢掉 ——
    //  所以这个宽度是世界单位、恒定，不会随半径一起放大。
    inline constexpr float S_BAND_W = 3.0f;

    //  正对度下限：平的环正侧看会退化成一条线。0 = 纯的（侧看即消失）；
    //  0.35 保住侧视时的可见度，代价是"平面感"弱一点。
    inline constexpr float S_FACING_FLOOR = 0.35f;

    // ===== 窗口包络：两端必须归零 ==========================================
    //  能量律在窗口两端都不是 0，硬切一定看得见（实测：去程终点 r = 0 时能量是满值，
    //  结果最亮的一刻被一刀砍掉）。四个数分开给 —— 两道波的高潮在不同的端：
    //    去程的高潮在【终点】（收到圆心那一刻最亮）→ 收尾要短，否则把 climax 抹平；
    //    抵达的高潮在【起点】（圆心最亮）→ 淡入要短、收尾要长，那才是"消散"。
    inline constexpr float S_OUT_RISE_F = 0.25f;
    inline constexpr float S_OUT_FALL_F = 0.12f;
    inline constexpr float S_IN_RISE_F = 0.10f;
    inline constexpr float S_IN_FALL_F = 0.40f;

    //  扭曲幅度的收尾占比：它【不】乘上面那条亮度包络（两者同步变淡会失去"亮脊当边界"的读感，
    //  见计划书 §9.4），但也不能在窗口边界硬停 —— 所以给一条【更晚、更短】的纯收尾。
    //  前 (1 - S_DIST_FALL_F) 段保持满幅，最后这一段平滑归零，末端恰好为 0。
    //  ⚠️ 用它的那个函数 shockDistortRamp 必须写在 shockProgress 之后（见文件末尾）。
    //  ⚠️ 不变量：riseF + S_DIST_FALL_F <= 1（超了淡入淡出会重叠，峰值到不了 1）。
    //     现在去程 0.25 + 0.15 = 0.40、抵达 0.10 + 0.15 = 0.25，安全。
    inline constexpr float S_DIST_FALL_F = 0.15f;

    //  介质被推的方向 = 波前传播的方向（冲击波经过时把介质顺着推走）。
    //  去程波向内收 → 推向内；抵达波向外散 → 推向外。两者互为反号，别写反。
    inline constexpr float S_OUT_PUSH = -1.0f;
    inline constexpr float S_IN_PUSH = +1.0f;

    // ===== 随时间变化的三件东西 ============================================
    //  归一化进度，两端钳死。
    inline float shockProgress(float tau, float t0, float t1)
    {
        return dClamp01((tau - t0) / (t1 - t0));
    }

    //  去程半径：单调收到 0（匀速 —— 水波不缓动）。
    inline float shockRadiusOut(float tau)
    {
        return S_OUT_R0 * (1.0f - shockProgress(tau, S_OUT_T0, S_OUT_T1));
    }

    //  抵达半径：单调散到 S_IN_R1（匀速）。
    inline float shockRadiusIn(float tau)
    {
        return S_IN_R1 * shockProgress(tau, S_IN_T0, S_IN_T1);
    }

    //  两道波的窗口互不相交，所以同一时刻最多一道活着。
    inline bool shockOutLive(float tau) { return tau >= S_OUT_T0 && tau < S_OUT_T1; }
    inline bool shockInLive(float tau) { return tau >= S_IN_T0 && tau < S_IN_T1; }

    //  能量律：I(r) = clamp((S_REF_R / r)^P, LO, HI)，P = S_FALL_POW。
    //  r 越小越亮（聚合）、越大越暗（扩散）。r → 0 会发散，所以两端都要钳。
    inline float shockEnergy(float r)
    {
        const float q = S_REF_R / (r > 1e-3f ? r : 1e-3f);
        const float k = std::pow(q, S_FALL_POW);   // 平面圆环：二维圆波，P = 1
        return (k < S_ENERGY_LO) ? S_ENERGY_LO : ((k > S_ENERGY_HI) ? S_ENERGY_HI : k);
    }

    //  扭曲的径向剖面：以波前（eps = 0）为中心的单峰（高斯），eps 是世界单位。
    inline float shockProfile(float eps)
    {
        const float x = eps / S_PROFILE_W;
        return std::exp(-x * x);
    }

    // ===== 窗口包络：返回 [0,1]，两端精确为 0 =============================
    //  T 形：前 S_*_RISE_F 段淡入，后 S_*_FALL_F 段淡出。
    inline float shockWindowOut(float tau)
    {
        const float p = shockProgress(tau, S_OUT_T0, S_OUT_T1);
        const float up = dSmooth(dClamp01(p / S_OUT_RISE_F));
        const float dn = dSmooth(dClamp01((p - (1.0f - S_OUT_FALL_F)) / S_OUT_FALL_F));
        return up * (1.0f - dn);
    }

    inline float shockWindowIn(float tau)
    {
        const float p = shockProgress(tau, S_IN_T0, S_IN_T1);
        const float up = dSmooth(dClamp01(p / S_IN_RISE_F));
        const float dn = dSmooth(dClamp01((p - (1.0f - S_IN_FALL_F)) / S_IN_FALL_F));
        return up * (1.0f - dn);
    }

    //  扭曲幅度的收尾曲线：前 (1 - S_DIST_FALL_F) 段为 1，最后 S_DIST_FALL_F 段平滑降到 0。
    //  必须放在 shockProgress 之后 —— 名字按定义点查找。
    //inline float shockDistortRamp(float tau, float t0, float t1)
    //{
    //    const float p = shockProgress(tau, t0, t1);
    //    const float dn = dSmooth(dClamp01((p - (1.0f - S_DIST_FALL_F)) / S_DIST_FALL_F));
    //    return 1.0f - dn;
    //}

    //  扭曲幅度的包络：起点与【该道波自己的】亮带淡入同步（避免"还没见光、背景先扭"），
    //  终点则比亮度更晚、更短地自己收掉（保留"光淡了、扰动还在"的读感）。末端恰好为 0。
    inline float shockDistortRamp(float tau, float t0, float t1, float riseF)
    {
        const float p = shockProgress(tau, t0, t1);
        const float up = dSmooth(dClamp01(p / riseF));
        const float dn = dSmooth(dClamp01((p - (1.0f - S_DIST_FALL_F)) / S_DIST_FALL_F));
        return up * (1.0f - dn);
    }
}

// ===== GL 侧：实现在 WarpDebris.cpp =====================================
//  与碎屑/光柱共用同一个 .cpp（避开 vcxproj 登记那一关）。
//  axisModel 的 Y 轴就是那道波的中心轴，原点就是波的原点：
//    去程传 shipWireModel；抵达传光柱那张面（pillarPlane）。
class Shader;
void WarpShockDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR, const glm::mat4& axisModel);

//  屏幕空间扭曲：渲染到 pingpongFBO[0]，再 blit 回 hdrColorBuffer —— 于是后处理链读到的是
//  扭曲后的画面，连 bloom 也从它里面提，波带的光晕会跟着一起扭。
//  screenW/H 是像素尺寸；quadVAO 是全屏四边形（属性布局与 bright_pass_ver 相同）。
void WarpShockDistort(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::mat4& axisModel, int screenW, int screenH, unsigned int quadVAO);