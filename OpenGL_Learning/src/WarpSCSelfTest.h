#pragma once
// ---------------------------------------------------------------------------
//  WarpSCSelfTest.h —— WarpSC.h（SC 分支时间线）的启动自测（P1–P7）
//
//  与 WarpSelfTest.h 逐行一致，只改了四处：include 换成 WarpSC.h、命名空间
//  换成 warp_sc_test、输出标签换成 [warp_sc]、硬失败宏换成 WARP_SC_TEST_HARD_FAIL。
//  另有一处真修复：P1 的扫描上界原本写死 1.22f（T_TOTAL=1.25 时代的余量值），
//  现改为 warp_sc::T_TOTAL - 0.03f，改时长时自动跟随。
//
//  沿用 PhysicsSelfTest.h 那套"header-only + 自 gate"约定：真实实现包在
//  #ifdef DEBUG_PHYS 内，#else 提供一个空的 inline void Run()，于是调用点
//  不需要任何 #ifdef，也不必改 vcxproj 的预处理器定义。
//
//  调用点：main 第一行，与 phys_test::Run(); 并列。
// ---------------------------------------------------------------------------

#ifdef DEBUG_PHYS

#include <cstdio>
#include <cmath>
#include <cassert>
#include <glm/glm.hpp>
#include "WarpSC.h"
#include "Spaceship.h"

#define WARP_SC_TEST_HARD_FAIL 0

namespace warp_sc_test
{
    inline int gFail = 0;

    // 打印一条断言结果。ok 为假时记一次失败并打印实测值与期望值。
    inline void Check(bool ok, const char* name, float got, float want, float tol)
    {
        if (ok)
        {
            std::printf("[warp_sc] %-52s OK\n", name);
            return;
        }
        ++gFail;
        std::printf("[warp_sc] %-52s FAIL   got=%.9g want=%.9g tol=%.3g\n",
            name, (double)got, (double)want, (double)tol);
#if WARP_SC_TEST_HARD_FAIL
        assert(false);
#endif
    }

    // 没有数值可比时的断言（存在性、唯一性一类）。
    inline void CheckTrue(bool ok, const char* name)
    {
        if (ok)
        {
            std::printf("[warp_sc] %-52s OK\n", name);
            return;
        }
        ++gFail;
        std::printf("[warp_sc] %-52s FAIL\n", name);
#if WARP_SC_TEST_HARD_FAIL
        assert(false);
#endif
    }

    // ===== P1 —— 命题 1：可见性不中断 =====================================
    //  区间取开区间 (0, T_TOTAL - 0.03)。端点处五通道本就同时为零，那里舰体走
    //  常规不透明路径、尚未发生任何跳变，不构成缺陷（计划书 注 2）。
    //  上界由常量推导，不要写死数字：写死的话改时长之后会静默丢掉尾段覆盖。
    inline void P1()
    {
        const float step = 0.001f;
        const float tEnd = warp_sc::T_TOTAL - 0.03f;   // 通道全部归零的时刻
        float worst = 1e9f, worstTau = -1.0f;

        for (float t = step; t < tEnd; t += step)
        {
            const warp_sc::Channels c = warp_sc::sample(t);
            float m = c.c;
            if (c.v > m) m = c.v;
            if (c.g > m) m = c.g;
            if (c.w > m) m = c.w;
            if (c.a > m) m = c.a;

            if (m < worst) { worst = m; worstTau = t; }
        }

        Check(worst > 0.0f, "P1  max(u_c,u_v,u_g,u_w,u_a) > 0 on the interval", worst, 0.0f, 0.0f);
        if (worst <= 0.0f)
            std::printf("      最暗的一帧出现在 tau = %.4f\n", (double)worstTau);
    }

    // ===== P2 —— 注 1：峰值条件 ===========================================
    //  每行必须满足 r1 <= f0，否则上升段与下降段在 (f0, r1) 上相乘、峰值达不到 1，
    //  而且不会报任何错。峰值取平台 [r1, f0] 的中点。
    inline void P2()
    {
        struct Row { const char* name; float r0, r1, f0, f1; };
        const Row rows[4] =
        {
            { "u_c", warp_sc::C_R0, warp_sc::C_R1, warp_sc::C_F0, warp_sc::C_F1 },
            { "u_v", warp_sc::V_R0, warp_sc::V_R1, warp_sc::V_F0, warp_sc::V_F1 },
            { "u_w", warp_sc::W_R0, warp_sc::W_R1, warp_sc::W_F0, warp_sc::W_F1 },
            { "u_a", warp_sc::A_R0, warp_sc::A_R1, warp_sc::A_F0, warp_sc::A_F1 },
        };

        for (int i = 0; i < 4; ++i)
        {
            char buf[96];

            std::snprintf(buf, sizeof(buf), "P2  %s: r1 <= f0", rows[i].name);
            CheckTrue(rows[i].r1 <= rows[i].f0, buf);

            const float mid = 0.5f * (rows[i].r1 + rows[i].f0);
            const float peak = warp_sc::trap(mid, rows[i].r0, rows[i].r1, rows[i].f0, rows[i].f1);

            std::snprintf(buf, sizeof(buf), "P2  %s: peak == 1", rows[i].name);
            Check(std::fabs(peak - 1.0f) < 1e-6f, buf, peak, 1.0f, 1e-6f);
        }
    }

    // ===== P3 —— 命题 3：任意帧率下峰值必被采到 ===========================
    //  30 fps 下时钟每帧走 1/30 s 而闪现上升段只有 0.05 s，不做钉住就会整帧跨过
    //  峰值。这里断言三件事：存在一帧 u_g == 1、传送恰好发生一次、且传送就在
    //  那一帧上。
    inline void P3()
    {
        const float dts[3] = { 1.0f / 30.0f, 1.0f / 60.0f, 1.0f / 144.0f };

        for (int i = 0; i < 3; ++i)
        {
            float tau = 0.0f;
            int peakFrames = 0, teleportFrames = 0;
            bool teleportOnPeak = false;

            for (int f = 0; f < 100000; ++f)
            {
                const warp_sc::Step s = warp_sc::advance(tau, dts[i]);
                const float g = warp_sc::sample(tau).g;

                if (g >= 1.0f)
                {
                    ++peakFrames;
                    if (s.teleport) teleportOnPeak = true;
                }
                if (s.teleport) ++teleportFrames;
                if (s.finished) break;
            }

            char buf[96];
            std::snprintf(buf, sizeof(buf), "P3  dt=1/%.0f: u_g reaches 1", 1.0f / dts[i]);
            CheckTrue(peakFrames >= 1, buf);

            std::snprintf(buf, sizeof(buf), "P3  dt=1/%.0f: teleport once", 1.0f / dts[i]);
            CheckTrue(teleportFrames == 1, buf);

            std::snprintf(buf, sizeof(buf), "P3  dt=1/%.0f: teleport on the peak frame", 1.0f / dts[i]);
            CheckTrue(teleportOnPeak, buf);
        }
    }

    // ===== P4 —— 命题 2：位移不变式 =======================================
    //  舰体与相机同时加同一个 delta，K - P 不变。实数上精确，浮点下不精确，
    //  所以必须给容差。
    inline void P4()
    {
        const glm::vec3 P(12.5f, -3.25f, 88.0f);
        const glm::vec3 K(-40.0f, 10.0f, 200.0f);
        const glm::vec3 d(31.0f, -17.5f, -62.75f);

        const float before = glm::length(K - P);
        const float after = glm::length((K + d) - (P + d));

        Check(std::fabs(after - before) < 1e-5f,
            "P4  |(K+d)-(P+d)| == |K-P|", after, before, 1e-5f);
    }

    // ===== P5 —— §5.1：传送不可被插值涂抹 =================================
    //  前半句可以逐位比：WarpTo 把同一个值写进 position 与 prevPosition。
    //  后半句必须用容差：glm::mix(p, p, a) 会先算 p*(1-a) 与 p*a 再相加，
    //  各自舍入之后不保证还原 p。
    inline void P5()
    {
        Spaceship s;
        const glm::vec3 target(5.5f, -2.25f, 70.125f);
        s.WarpTo(target);

        const bool posOk = (s.position.x == target.x)
            && (s.position.y == target.y)
            && (s.position.z == target.z);
        CheckTrue(posOk, "P5  WarpTo writes position");

        const bool prevOk = (s.prevPosition.x == s.position.x)
            && (s.prevPosition.y == s.position.y)
            && (s.prevPosition.z == s.position.z);
        CheckTrue(prevOk, "P5  prevPosition == position (bitwise)");

        const float alphas[3] = { 0.0f, 0.5f, 0.999f };
        for (int i = 0; i < 3; ++i)
        {
            const float err = glm::length(s.RenderPosition(alphas[i]) - s.position);
            char buf[96];
            std::snprintf(buf, sizeof(buf), "P5  RenderPosition(a=%.3f) == P", (double)alphas[i]);
            Check(err < 1e-4f, buf, err, 0.0f, 1e-4f);
        }
    }

    // ===== P6 —— §3.3：两端的导数条件 ====================================
    //  用单侧差分而不是解析式，才能查出实现被写错（例如 φ 的系数抄错）。
    //  阈值按余项量级取：一阶单侧差分在端点处的余项是 3h = 3e-3，
    //  二阶单侧差分在 0 处对 σ 给 6 - 12h、对 φ 给 60h - 210h²。
    inline void P6()
    {
        const float h = 1e-3f;

        const float s0 = (warp_sc::smoothstep01(h) - warp_sc::smoothstep01(0.0f)) / h;
        const float s1 = (warp_sc::smoothstep01(1.0f) - warp_sc::smoothstep01(1.0f - h)) / h;
        const float q0 = (warp_sc::quintic01(h) - warp_sc::quintic01(0.0f)) / h;
        const float q1 = (warp_sc::quintic01(1.0f) - warp_sc::quintic01(1.0f - h)) / h;

        Check(std::fabs(s0) < 1e-2f, "P6  sigma'(0) ~ 0", s0, 0.0f, 1e-2f);
        Check(std::fabs(s1) < 1e-2f, "P6  sigma'(1) ~ 0", s1, 0.0f, 1e-2f);
        Check(std::fabs(q0) < 1e-2f, "P6  phi'(0)   ~ 0", q0, 0.0f, 1e-2f);
        Check(std::fabs(q1) < 1e-2f, "P6  phi'(1)   ~ 0", q1, 0.0f, 1e-2f);

        const float s2 = (warp_sc::smoothstep01(2.0f * h) - 2.0f * warp_sc::smoothstep01(h)
            + warp_sc::smoothstep01(0.0f)) / (h * h);
        const float q2 = (warp_sc::quintic01(2.0f * h) - 2.0f * warp_sc::quintic01(h)
            + warp_sc::quintic01(0.0f)) / (h * h);

        // σ''(0) = 6（不是 C2），φ''(0) = 0（是 C2）—— 这一对比才是判别式。
        Check(std::fabs(s2 - 6.0f) < 1e-1f, "P6  sigma''(0) ~ 6  => not C2", s2, 6.0f, 1e-1f);
        Check(std::fabs(q2) < 2e-1f, "P6  phi''(0)   ~ 0  => is C2", q2, 0.0f, 2e-1f);
    }

    // ===== P7 —— §1.1 第 2 条：实际用时的量化上界 =========================
    //  偏差只有两个来源：逐帧采样的取整最多多花一帧，τ* 钉住再多花一帧，
    //  所以实际用时落在 [T_Σ, T_Σ + 2*dt) 内。30 fps 下上界是 6.7% 的绝对量、
    //  相对 T_TOTAL 约 3.3%，这是逐帧采样的固有代价，不是缺陷。
    inline void P7()
    {
        const float dts[3] = { 1.0f / 30.0f, 1.0f / 60.0f, 1.0f / 144.0f };

        for (int i = 0; i < 3; ++i)
        {
            const float dt = dts[i];
            float tau = 0.0f, wall = 0.0f;
            int pinFrames = 0;

            for (int f = 0; f < 100000; ++f)
            {
                const warp_sc::Step s = warp_sc::advance(tau, dt);
                wall += dt;
                if (s.teleport) ++pinFrames;
                if (s.finished) break;
            }

            char buf[96];
            std::snprintf(buf, sizeof(buf), "P7  dt=1/%.0f: wall time in [T_S, T_S+2dt)", 1.0f / dt);
            CheckTrue(wall >= warp_sc::T_TOTAL && wall < warp_sc::T_TOTAL + 2.0f * dt, buf);

            std::snprintf(buf, sizeof(buf), "P7  dt=1/%.0f: pinned exactly once", 1.0f / dt);
            CheckTrue(pinFrames == 1, buf);
        }
    }

    inline void Run()
    {
        gFail = 0;

        std::printf("\n===== warp_sc self test (P1..P7) =====\n");
        P1(); P2(); P3(); P4(); P5(); P6(); P7();

        if (gFail == 0) std::printf("===== warp_sc self test: all OK =====\n\n");
        else            std::printf("===== warp_sc self test: %d FAILED =====\n\n", gFail);
    }
}

#else   // !DEBUG_PHYS —— 非 Debug 配置下不产生任何代码

namespace warp_sc_test { inline void Run() {} }

#endif#pragma once
