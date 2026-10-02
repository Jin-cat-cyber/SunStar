#pragma once
// ---------------------------------------------------------------------------
//  WarpDebris.h —— SC 分支起飞段的块状碎屑：纯运动学，不含 glm、不含 GL。
//
//  与 WarpSC.h 分开是有意的：WarpSC.h 是 Warp.h 的逐行副本、要能对照着看，
//  碎屑是 SC 分支独有的新东西，塞进去会破坏那份对照。
//
//  一切由【唯一时钟 tau】与一个【按碎片序号确定的哈希】驱动：没有跨帧状态，
//  也不碰 std::rand 的全局状态。位置是 tau 的解析函数而不是累加出来的，
//  所以同一趟折跃每次看都一样，也不会因为帧率不同而走样。
//
//  坐标一律用【世界单位】，且都定在"折跃开始那一帧的舰体局部系"里。
//  舰体局部坐标比世界大 2000 倍（GetModelMatrix 里有 scale(0.0005)），
//  调用方必须先把锚点矩阵还原到世界尺度（乘 scale(1/0.0005)）再乘本文件的坐标。
//  舰体轴向约定：局部 -Y 是舰首，+Y 是舰尾，"向后甩"就是 +Y 方向。
// ---------------------------------------------------------------------------
#include "WarpSC.h"

#include <cmath>
#include <glm/glm.hpp>

namespace warp_sc
{
    // ===== 碎屑观感参数（最后统一调参时改这里）=============================
    //inline constexpr int   D_COUNT = 56;                     // 碎片数
    //inline constexpr float D_T0 = 0.25f;                  // 最早出生时刻（秒）
    //inline constexpr float D_T1 = TAU_STAR - 0.10f;       // 最晚出生时刻
    //inline constexpr float D_LIFE_MIN = 0.55f;                  // 寿命下界
    //inline constexpr float D_LIFE_MAX = 0.95f;                  // 寿命上界
    //inline constexpr float D_FADE_IN = 0.06f;                  // 淡入时长
    //inline constexpr float D_FADE_OUT = 0.25f;                  // 淡出时长
    //inline constexpr float D_SIZE_MIN = 0.60f;                  // 半尺寸下界（世界单位）
    //inline constexpr float D_SIZE_MAX = 1.60f;                  // 半尺寸上界
    //inline constexpr float D_FLAT_K = 0.30f;                  // 矩形板被压扁那一轴的倍率
    //inline constexpr float D_FLAT_P = 0.60f;                  // 取到矩形板的概率
    //inline constexpr float D_OUT_MIN = 3.0f;                   // 横向速度参数下界
    //inline constexpr float D_OUT_MAX = 9.0f;                   // 横向速度参数上界
    //inline constexpr float D_BACK_MIN = 1.0f;                   // 沿局部 +Y 向后的速度参数下界
    //inline constexpr float D_BACK_MAX = 4.0f;                   // 上界
    ////inline constexpr float D_SPIN_MAX = 3.0f;                   // 自转角速度上界（弧度/秒）
    //inline constexpr float D_ROD_P = 0.35f;                  // 取到"长条"的概率（某一轴拉长）
    //inline constexpr float D_ROD_K = 2.6f;                   // 长条那一轴的拉长倍率
    //inline constexpr float D_SPAWN_L = 0.95f;                  // 出生点轴向半程（× 舰体半径）
    //inline constexpr float D_SPAWN_W = 0.30f;                  // 出生点横向半程（× 舰体半径）
    inline constexpr int   D_COUNT = 72;                     // 碎片数
    inline constexpr float D_T0 = 0.35f;                  // 最早出生时刻（秒）
    inline constexpr float D_T1 = TAU_STAR - 0.10f;       // 最晚出生时刻
    inline constexpr float D_LIFE_MIN = 0.40f;                  // 寿命下界
    inline constexpr float D_LIFE_MAX = 0.70f;                  // 寿命上界
    inline constexpr float D_FADE_IN = 0.04f;                  // 淡入时长
    inline constexpr float D_FADE_OUT = 0.16f;                  // 淡出时长
    inline constexpr float D_SIZE_MIN = 0.45f;                  // 半尺寸下界（世界单位）
    inline constexpr float D_SIZE_MAX = 1.25f;                  // 半尺寸上界
    inline constexpr float D_FLAT_K = 0.22f;                  // 矩形板被压扁那一轴的倍率
    inline constexpr float D_FLAT_P = 0.45f;                  // 取到矩形板的概率
    inline constexpr float D_OUT_MIN = 7.0f;                   // 横向速度参数下界
    inline constexpr float D_OUT_MAX = 20.0f;                  // 横向速度参数上界
    inline constexpr float D_BACK_MIN = 2.0f;                   // 沿局部 +Y 向后的速度参数下界
    inline constexpr float D_BACK_MAX = 8.0f;                   // 上界
    //inline constexpr float D_SPIN_MAX = 3.0f;                   // 自转角速度上界（弧度/秒）
    inline constexpr float D_ROD_P = 0.45f;                  // 取到"长条"的概率（某一轴拉长）
    inline constexpr float D_ROD_K = 3.2f;                   // 长条那一轴的拉长倍率
    inline constexpr float D_SPAWN_L = 0.95f;                  // 出生点轴向半程（× 舰体半径）
    inline constexpr float D_SPAWN_W = 0.30f;                  // 出生点横向半程（× 舰体半径）
    //  最后一个碎片消亡的时刻：绘制门控用它，免得每帧都白算一遍。
    inline constexpr float D_END = D_T1 + D_LIFE_MAX;

    // ===== 整数哈希：把 (序号, 通道) 打散到 [0,1) ==========================
    //  乘法-移位混合（MurmurHash3 的 finalizer），纯整数运算、无状态。
    inline unsigned int dMix(unsigned int x)
    {
        x ^= x >> 16; x *= 0x7feb352du;
        x ^= x >> 15; x *= 0x846ca68bu;
        x ^= x >> 16;
        return x;
    }

    inline float dRand(int i, int k)
    {
        const unsigned int h = dMix(static_cast<unsigned int>(i) * 0x9e3779b9u
            + static_cast<unsigned int>(k) * 0x85ebca6bu);
        // 只取低 24 位：float 的尾数就 24 位，再多的位也表达不出来。
        return static_cast<float>(h & 0x00ffffffu) / 16777216.0f;
    }

    inline float dLerp(float a, float b, float x) { return a + (b - a) * x; }
    inline float dClamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }
    inline float dSmooth(float x) { x = dClamp01(x); return x * x * (3.0f - 2.0f * x); }

    // ===== 一个碎片 ========================================================
    struct Debris
    {
        float t0;        // 出生时刻（秒）
        float life;      // 寿命（秒）：走完位移所用的时间，也是淡出时刻
        float p0[3];     // 出生点（世界单位，折跃起始帧的舰体局部系）
        float v[3];      // 速度参数：终位移 = v * life，初速 = 2 * v
        //float axis[3];   // 自转轴（单位向量）
        //float spin;      // 自转角速度（弧度/秒）
        //float half[3];   // 半尺寸（世界单位）：某一轴被压扁就是矩形板
        float half[3];   // 半尺寸（世界单位）：三轴都是舰体局部轴，拉长或压扁就得到长条与平板
        float tint;      // 色偏，0 = 偏青、1 = 偏白
    };

    inline Debris debrisAt(int i, float bowR)
    {
        Debris d;

        d.t0 = dLerp(D_T0, D_T1, dRand(i, 1));
        d.life = dLerp(D_LIFE_MIN, D_LIFE_MAX, dRand(i, 2));

        // 出生点：舰体包围盒内的一点。轴向（局部 Y）用整个半长，横向只取三成 ——
        // 舰体是细长的，横向铺满包围盒会让碎片像从空气里冒出来。
        for (int k = 0; k < 3; ++k)
        {
            const float s = (k == 1) ? D_SPAWN_L : D_SPAWN_W;
            d.p0[k] = (dRand(i, 3 + k) * 2.0f - 1.0f) * s * bowR;
        }

        // 飞出方向：只取舰体的【六个局部方向】之一（±X / ±Y / ±Z）——
        // 横向、纵向、竖向都是舰体自己的轴，所以碎片不会斜着飞。
        // 全零初始化在前，再往选中的那一轴上写，避免留下未写的分量。
        const int   outAxis = static_cast<int>(dRand(i, 8) * 3.0f) % 3;
        const float outSign = (dRand(i, 9) < 0.5f) ? -1.0f : 1.0f;
        const float outSpd = dLerp(D_OUT_MIN, D_OUT_MAX, dRand(i, 10));
        d.v[0] = 0.0f; d.v[1] = 0.0f; d.v[2] = 0.0f;
        d.v[outAxis] = outSign * outSpd;
        // 再沿局部 +Y 叠一点"被落在后面"的分量：舰首在 -Y，所以 +Y 是向后。
        d.v[1] += dLerp(D_BACK_MIN, D_BACK_MAX, dRand(i, 11));

        // 尺寸：先三轴独立取基线，再按类型改一轴 ——
        //   长条：某一轴拉长 D_ROD_K 倍；
        //   平板：某一轴压扁 D_FLAT_K 倍；
        //   其余：方块。
        // 无论哪一类，三个轴都是舰体局部轴，朝向天然与舰体对齐。
        for (int k = 0; k < 3; ++k)
            d.half[k] = dLerp(D_SIZE_MIN, D_SIZE_MAX, dRand(i, 12 + k));
        const float kind = dRand(i, 15);
        if (kind < D_ROD_P)
            d.half[static_cast<int>(dRand(i, 16) * 3.0f) % 3] *= D_ROD_K;
        else if (kind < D_ROD_P + D_FLAT_P)
            d.half[static_cast<int>(dRand(i, 17) * 3.0f) % 3] *= D_FLAT_K;

        d.tint = dRand(i, 18);
        return d;
    }

    //inline Debris debrisAt(int i, float bowR)
    //{
    //    Debris d;

    //    d.t0 = dLerp(D_T0, D_T1, dRand(i, 1));
    //    d.life = dLerp(D_LIFE_MIN, D_LIFE_MAX, dRand(i, 2));

    //    // 出生点：舰体包围盒内的一点。轴向（局部 Y）用整个半长，横向只取三成 ——
    //    // 舰体是细长的，横向铺满包围盒会让碎片像从空气里冒出来。
    //    for (int k = 0; k < 3; ++k)
    //    {
    //        const float s = (k == 1) ? D_SPAWN_L : D_SPAWN_W;
    //        d.p0[k] = (dRand(i, 3 + k) * 2.0f - 1.0f) * s * bowR;
    //    }

    //    // 速度：横向（局部 XZ 平面）朝外，外加沿局部 +Y 向后甩 ——
    //    // 舰首在 -Y，所以 +Y 就是"被落在后面"的方向。
    //    float ox = d.p0[0], oz = d.p0[2];
    //    float rl = std::sqrt(ox * ox + oz * oz);
    //    if (rl < 1e-4f)     // 退化：出生点几乎落在轴线上，用哈希方向顶上
    //    {
    //        const float ang = dRand(i, 8) * 6.2831853f;
    //        ox = std::cos(ang); oz = std::sin(ang); rl = 1.0f;
    //    }
    //    const float out = dLerp(D_OUT_MIN, D_OUT_MAX, dRand(i, 9));
    //    const float back = dLerp(D_BACK_MIN, D_BACK_MAX, dRand(i, 10));
    //    d.v[0] = ox / rl * out;
    //    d.v[1] = back;
    //    d.v[2] = oz / rl * out;

    //    // 自转轴：单位化之前先兜一次零向量（三分量同时接近零虽然概率极低）。
    //    float ax = dRand(i, 11) * 2.0f - 1.0f;
    //    float ay = dRand(i, 12) * 2.0f - 1.0f;
    //    float az = dRand(i, 13) * 2.0f - 1.0f;
    //    float al = std::sqrt(ax * ax + ay * ay + az * az);
    //    if (al < 1e-4f) { ax = 0.0f; ay = 1.0f; az = 0.0f; al = 1.0f; }
    //    d.axis[0] = ax / al; d.axis[1] = ay / al; d.axis[2] = az / al;
    //    d.spin = (dRand(i, 14) * 2.0f - 1.0f) * D_SPIN_MAX;

    //    // 尺寸：三轴独立取，再按概率压扁其中一轴得到矩形板。
    //    for (int k = 0; k < 3; ++k)
    //        d.half[k] = dLerp(D_SIZE_MIN, D_SIZE_MAX, dRand(i, 15 + k));
    //    if (dRand(i, 18) < D_FLAT_P)
    //    {
    //        const int k = static_cast<int>(dRand(i, 19) * 3.0f) % 3;
    //        d.half[k] *= D_FLAT_K;
    //    }

    //    d.tint = dRand(i, 20);
    //    return d;
    //}

    // ===== 随时间变化的两件东西 ============================================
    //  进度：0 = 刚出生，1 = 走完寿命。位置与亮度共用它，两者天然同步。
    inline float debrisPhase(const Debris& d, float tau)
    {
        return dClamp01((tau - d.t0) / (d.life > 1e-4f ? d.life : 1e-4f));
    }

    //  位移倍率 b(x) = 2x - x^2：起步快、末尾停。碎屑是"被甩出去的"，
    //  初速必须非零，末速为 0 才不会一直飞下去。b(0)=0, b'(0)=2, b(1)=1, b'(1)=0，
    //  于是：初速 = 2 * v，终位移 = v * life。
    inline float debrisBurst(float x) { return x * (2.0f - x); }

    //  亮度：淡入乘淡出。两段都用 σ（C1），所以刚出生与刚消亡时都不会跳亮。
    inline float debrisAlpha(const Debris& d, float tau)
    {
        const float t = tau - d.t0;
        if (t <= 0.0f) return 0.0f;
        const float up = dSmooth(t / D_FADE_IN);
        const float down = dSmooth((t - (d.life - D_FADE_OUT)) / D_FADE_OUT);
        return up * (1.0f - down);
    }
}


// ===== GL 侧：实现在 WarpDebris.cpp =====================================
//  这里只声明，不把 Shader.h / glad 拉进本头 —— 只用到 Shader 的引用。
class Shader;   // 见 Shader.h

//  主循环之前调用一次：建立方体网格与实例缓冲。
void WarpDebrisInit();

//  每帧在 shipModel 算完之后调用：折跃开始时记下锚点姿态，整趟不变。
void WarpDebrisAnchor(const glm::mat4& shipModel);

//  起飞段碎屑的绘制。没有碎片可见时它自己什么都不做（含 GL 状态的设置与还原）。
void WarpDebrisDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR);