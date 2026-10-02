#pragma once
// ---------------------------------------------------------------------------
//  WarpSelfTest.h —— 折跃时间线的启动自测（P1–P7）
//
//  沿用 PhysicsSelfTest.h 那套"header-only + 自 gate"约定：真实实现包在
//  #ifdef DEBUG_PHYS 内，#else 提供一个空的 inline void Run()，于是调用点
//  不需要任何 #ifdef，也不必改 vcxproj 的预处理器定义。
//
//  调用点：main 第一行，与 phys_test::Run(); 并列。
//  每条断言的设计依据见 Tech_finding_Report/战列巡航舰折跃_设计与实现计划书.md §10.1。
//
//  置 WARP_TEST_HARD_FAIL 为 1 可让失败直接触发 assert（Debug 下当场断下来）；
//  默认只打印，与 phys_test 的行为一致。
// ---------------------------------------------------------------------------

#ifdef DEBUG_PHYS

#include <cstdio>
#include <cmath>
#include <cassert>
#include <glm/glm.hpp>
#include "Warp.h"
#include "Spaceship.h"

#define WARP_TEST_HARD_FAIL 0

namespace warp_test
{
    inline int gFail = 0;

    // 打印一条断言结果。ok 为假时记一次失败并打印实测值与期望值。
    inline void Check(bool ok, const char* name, float got, float want, float tol)
    {
        if (ok)
        {
            std::printf("[warp] %-52s OK\n", name);
            return;
        }
        ++gFail;
        std::printf("[warp] %-52s FAIL   got=%.9g want=%.9g tol=%.3g\n",
            name, (double)got, (double)want, (double)tol);
#if WARP_TEST_HARD_FAIL
        assert(false);
#endif
    }

    // 没有数值可比时的断言（存在性、唯一性一类）。
    inline void CheckTrue(bool ok, const char* name)
    {
        if (ok)
        {
            std::printf("[warp] %-52s OK\n", name);
            return;
        }
        ++gFail;
        std::printf("[warp] %-52s FAIL\n", name);
#if WARP_TEST_HARD_FAIL
        assert(false);
#endif
    }

    // ===== P1 —— 命题 1：可见性不中断 =====================================
    //  区间取开区间 (0, 1.22)。τ = 0 与 τ > 1.22 处五通道本就同时为零，那两处
    //  舰体走常规不透明路径、尚未发生任何跳变，不构成缺陷（计划书 注 2）。
    inline void P1()
    {
        const float step = 0.001f;
        float worst = 1e9f, worstTau = -1.0f;

        for (float t = step; t < 1.22f; t += step)
        {
            const warp::Channels c = warp::sample(t);
            float m = c.c;
            if (c.v > m) m = c.v;
            if (c.g > m) m = c.g;
            if (c.w > m) m = c.w;
            if (c.a > m) m = c.a;

            if (m < worst) { worst = m; worstTau = t; }
        }

        Check(worst > 0.0f, "P1  max(u_c,u_v,u_g,u_w,u_a) > 0 on (0,1.22)", worst, 0.0f, 0.0f);
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
            { "u_c", warp::C_R0, warp::C_R1, warp::C_F0, warp::C_F1 },
            { "u_v", warp::V_R0, warp::V_R1, warp::V_F0, warp::V_F1 },
            { "u_w", warp::W_R0, warp::W_R1, warp::W_F0, warp::W_F1 },
            { "u_a", warp::A_R0, warp::A_R1, warp::A_F0, warp::A_F1 },
        };

        for (int i = 0; i < 4; ++i)
        {
            char buf[96];

            std::snprintf(buf, sizeof(buf), "P2  %s: r1 <= f0", rows[i].name);
            CheckTrue(rows[i].r1 <= rows[i].f0, buf);

            const float mid = 0.5f * (rows[i].r1 + rows[i].f0);
            const float peak = warp::trap(mid, rows[i].r0, rows[i].r1, rows[i].f0, rows[i].f1);

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
                const warp::Step s = warp::advance(tau, dts[i]);
                const float g = warp::sample(tau).g;

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

        const float s0 = (warp::smoothstep01(h) - warp::smoothstep01(0.0f)) / h;
        const float s1 = (warp::smoothstep01(1.0f) - warp::smoothstep01(1.0f - h)) / h;
        const float q0 = (warp::quintic01(h) - warp::quintic01(0.0f)) / h;
        const float q1 = (warp::quintic01(1.0f) - warp::quintic01(1.0f - h)) / h;

        Check(std::fabs(s0) < 1e-2f, "P6  sigma'(0) ~ 0", s0, 0.0f, 1e-2f);
        Check(std::fabs(s1) < 1e-2f, "P6  sigma'(1) ~ 0", s1, 0.0f, 1e-2f);
        Check(std::fabs(q0) < 1e-2f, "P6  phi'(0)   ~ 0", q0, 0.0f, 1e-2f);
        Check(std::fabs(q1) < 1e-2f, "P6  phi'(1)   ~ 0", q1, 0.0f, 1e-2f);

        const float s2 = (warp::smoothstep01(2.0f * h) - 2.0f * warp::smoothstep01(h)
            + warp::smoothstep01(0.0f)) / (h * h);
        const float q2 = (warp::quintic01(2.0f * h) - 2.0f * warp::quintic01(h)
            + warp::quintic01(0.0f)) / (h * h);

        // σ''(0) = 6（不是 C2），φ''(0) = 0（是 C2）—— 这一对比才是判别式。
        Check(std::fabs(s2 - 6.0f) < 1e-1f, "P6  sigma''(0) ~ 6  => not C2", s2, 6.0f, 1e-1f);
        Check(std::fabs(q2) < 2e-1f, "P6  phi''(0)   ~ 0  => is C2", q2, 0.0f, 2e-1f);
    }

    // ===== P7 —— §1.1 第 2 条：实际用时的量化上界 =========================
    //  偏差只有两个来源：逐帧采样的取整最多多花一帧，τ* 钉住再多花一帧，
    //  所以实际用时落在 [T_Σ, T_Σ + 2*dt) 内。30 fps 下上界是 5.3%，这是
    //  逐帧采样的固有代价，不是缺陷（计划书 §10.1 已写清）。
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
                const warp::Step s = warp::advance(tau, dt);
                wall += dt;
                if (s.teleport) ++pinFrames;
                if (s.finished) break;
            }

            char buf[96];
            std::snprintf(buf, sizeof(buf), "P7  dt=1/%.0f: wall time in [T_S, T_S+2dt)", 1.0f / dt);
            CheckTrue(wall >= warp::T_TOTAL && wall < warp::T_TOTAL + 2.0f * dt, buf);

            std::snprintf(buf, sizeof(buf), "P7  dt=1/%.0f: pinned exactly once", 1.0f / dt);
            CheckTrue(pinFrames == 1, buf);
        }
    }

    inline void Run()
    {
        gFail = 0;

        std::printf("\n===== warp self test (P1..P7) =====\n");
        P1(); P2(); P3(); P4(); P5(); P6(); P7();

        if (gFail == 0) std::printf("===== warp self test: all OK =====\n\n");
        else            std::printf("===== warp self test: %d FAILED =====\n\n", gFail);
    }
}

#else   // !DEBUG_PHYS —— 非 Debug 配置下不产生任何代码

namespace warp_test { inline void Run() {} }

#endif