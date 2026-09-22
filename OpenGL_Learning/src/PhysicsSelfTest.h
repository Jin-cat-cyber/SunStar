#pragma once
#include <cstdio>
#include "Spaceship.h"

// ===================== 定步长物理自测 =====================
// 用法：VS 项目属性 → C/C++ → 预处理器 → 预处理器定义，只在 Debug 配置加 DEBUG_PHYS
// Release 下整个模块退化成空函数，调用点无需 #ifdef
namespace phys_test
{
#ifdef DEBUG_PHYS

    constexpr int TOTAL_STEPS = 1200;      // 1200 步 = 10 秒 @120Hz

    // 输入脚本：只按「物理步序号」驱动 → 与帧切分、帧率都无关；10 秒覆盖一个完整循环
    inline void Script(Spaceship& s, int step)
    {
        s.targetSpeed = 0.0f;
        s.targetVerticalSpeed = 0.0f;
        s.targetYawAV = s.targetPitchAV = s.targetRollAV = 0.0f;
        switch ((step / 120) % 6) {
        case 0: s.targetSpeed = s.forwardMax; break;                                  // 加速前进
        case 1: s.targetSpeed = s.forwardMax; s.targetYawAV = s.turnRate; break;      // 前进 + 偏航
        case 2: s.targetSpeed = -s.backwardMax; s.targetRollAV = s.turnRate; break;   // 倒退 + 滚转
        case 3: s.targetPitchAV = s.turnRate; s.targetVerticalSpeed = s.upMax; break; // 俯仰 + 上升
        case 4: s.targetVerticalSpeed = -s.downMax; s.targetRollAV = -s.turnRate; break;
        default: break;                                                              // 滑行
        }
    }

    inline void Diff(const Spaceship& a, const Spaceship& b, float& dPos, float& dQuat)
    {
        glm::vec3 v = glm::abs(a.position - b.position);
        dPos = glm::max(v.x, glm::max(v.y, v.z));
        dQuat = glm::max(glm::abs(a.heading.x - b.heading.x),
            glm::max(glm::abs(a.heading.y - b.heading.y),
                glm::max(glm::abs(a.heading.z - b.heading.z),
                    glm::abs(a.heading.w - b.heading.w))));
    }

    struct RunOut { Spaceship ship; int frames = 0; float wallTime = 0.0f; };

    // 用给定帧时间模式推进到「恰好 totalSteps 步」
    // 注意：这里刻意手写步进循环而不用 AdvanceFixed —— 因为要把输入按"步序号"注入；
    //       累加器语义本身由 T2/T3/T4 直接调用共享的 AdvanceFixed 覆盖
    inline RunOut RunFrames(float frameDt, int totalSteps, bool jitter)
    {
        RunOut o;
        float accum = 0.0f, dt = frameDt;
        unsigned seed = 20260915u;                        // 固定种子 → 抖动可复现
        int step = 0;
        while (step < totalSteps) {
            if (jitter) {                                 // 确定性伪随机帧时间：0.5~8 个 FIXED_DT
                seed = seed * 1664525u + 1013904223u;
                float u = float((seed >> 8) & 0xFFFFFFu) / float(0xFFFFFFu);
                dt = (0.5f + 7.5f * u) * Spaceship::FIXED_DT;
            }
            o.wallTime += dt;
            accum += dt;
            if (accum > Spaceship::MAX_ACCUM) accum = Spaceship::MAX_ACCUM;
            while (accum >= Spaceship::FIXED_DT && step < totalSteps) {
                Script(o.ship, step);
                o.ship.FixedUpdate(Spaceship::FIXED_DT);
                accum -= Spaceship::FIXED_DT;
                ++step;
            }
            ++o.frames;
        }
        return o;
    }

    inline void Run()
    {
        printf("\n=========== 定步长物理自测 (DEBUG_PHYS) ===========\n");
        printf("FIXED_DT = %.6f s (%.1f Hz)   MAX_ACCUM = %.3f s   总步数 = %d\n\n",
            Spaceship::FIXED_DT, 1.0f / Spaceship::FIXED_DT, Spaceship::MAX_ACCUM, TOTAL_STEPS);

        // ---------- T1 帧模式无关：不同帧切分/帧率，同样步数下状态必须逐位一致 ----------
        struct Row { const char* name; float dt; bool jitter; };
        const Row rows[] = {
            { "120 fps (1 步/帧)", Spaceship::FIXED_DT,        false },
            { " 60 fps (2 步/帧)", Spaceship::FIXED_DT * 2.0f, false },
            { " 30 fps (4 步/帧)", Spaceship::FIXED_DT * 4.0f, false },
            { "144 fps           ", 1.0f / 144.0f,             false },
            { "240 fps           ", 1.0f / 240.0f,             false },
            { "抖动 0.5~8x DT    ", 0.0f,                      true  },
        };
        const int nRows = int(sizeof(rows) / sizeof(rows[0]));
        RunOut refRun = RunFrames(rows[0].dt, TOTAL_STEPS, false);
        printf("[T1] 帧模式无关（基准 = %s）\n", rows[0].name);
        printf("     %-20s %6s %9s %14s %14s  %s\n", "帧模式", "帧数", "墙钟(s)", "位置差", "四元数差", "判定");
        bool t1 = true;
        for (int i = 0; i < nRows; ++i) {
            RunOut r = (i == 0) ? refRun : RunFrames(rows[i].dt, TOTAL_STEPS, rows[i].jitter);
            float dp = 0.0f, dq = 0.0f;
            Diff(refRun.ship, r.ship, dp, dq);
            const bool ok = (dp == 0.0f && dq == 0.0f);
            if (i > 0) t1 = t1 && ok;
            printf("     %-20s %6d %9.4f %14.9f %14.9f  %s\n",
                rows[i].name, r.frames, r.wallTime, dp, dq, ok ? "PASS" : "FAIL");
        }
        printf("     >>> T1: %s\n\n", t1 ? "PASS（物理只由步数决定，与帧切分无关）" : "FAIL");

        // ---------- T2 卡顿钳制（直接调共享步进器） ----------
        {
            Spaceship s; float accum = 0.0f, alpha = 0.0f;
            const int steps = s.AdvanceFixed(accum, 3.0f, alpha);          // 单帧 3 秒
            const int expect = int(Spaceship::MAX_ACCUM / Spaceship::FIXED_DT);
            printf("[T2] 卡顿钳制: 单帧 3.0 s -> 执行 %d 步（期望 %d，不钳制会是 360）\n", steps, expect);
            printf("     >>> T2: %s\n\n", (steps >= expect - 1 && steps <= expect) ? "PASS" : "FAIL");
        }

        // ---------- T3 插值有效性：帧率与 120Hz 不同相时，未插值的渲染位置会一顿一顿 ----------
        {
            Spaceship s;
            for (int i = 0; i < 300; ++i) {          // 2.5 s 直线加速到最大速度（不转向 → Forward() 恒定）
                s.targetSpeed = s.forwardMax;
                s.FixedUpdate(Spaceship::FIXED_DT);
            }
            const float dt = 1.0f / 144.0f;          // 144 fps：与 120Hz 不同相
            const int   frames = 288;                // 2 秒
            float accum = 0.0f, alpha = 0.0f;
            glm::vec3 prevI(0.0f), prevR(0.0f);
            float mnI = 1e30f, mxI = -1e30f, sumI = 0.0f;
            float mnR = 1e30f, mxR = -1e30f, sumR = 0.0f;
            int n = 0;
            for (int f = 0; f < frames; ++f) {
                s.targetSpeed = s.forwardMax;        // 匀速：整帧输入不变，可在步进前设置
                s.AdvanceFixed(accum, dt, alpha);
                const glm::vec3 pI = s.RenderPosition(alpha);   // 插值后（渲染路径该用的）
                const glm::vec3 pR = s.position;                // 未插值（物理原始位置）
                if (n > 0) {
                    const float dI = glm::length(pI - prevI), dR = glm::length(pR - prevR);
                    mnI = glm::min(mnI, dI); mxI = glm::max(mxI, dI); sumI += dI;
                    mnR = glm::min(mnR, dR); mxR = glm::max(mxR, dR); sumR += dR;
                }
                prevI = pI; prevR = pR; ++n;
            }
            const float meanI = sumI / float(n), meanR = sumR / float(n);
            const float jitI = (mxI - mnI) / meanI, jitR = (mxR - mnR) / meanR;
            printf("[T3] 插值有效性（144 fps 匀速直线，%d 帧；一步位移 %.4f 单位）\n", n,
                s.forwardMax * Spaceship::FIXED_DT);
            printf("     插值后  逐帧位移 min=%.5f max=%.5f mean=%.5f -> 抖动=%.4f\n", mnI, mxI, meanI, jitI);
            printf("     未插值  逐帧位移 min=%.5f max=%.5f mean=%.5f -> 抖动=%.4f\n", mnR, mxR, meanR, jitR);
            printf("     >>> T3: %s（期望 插值<0.05 且 未插值>0.5）\n\n",
                (jitI < 0.05f && jitR > 0.5f) ? "PASS" : "FAIL");
        }

        // ---------- T4 共享步进器不变式（生产主循环走的就是这条路径） ----------
        {
            struct Case { const char* name; float dt; };
            const Case cases[] = {
                { "240 fps (0~1 步/帧)", Spaceship::FIXED_DT * 0.5f },
                { "120 fps (1 步/帧)  ", Spaceship::FIXED_DT       },
                { " 60 fps (2 步/帧)  ", Spaceship::FIXED_DT * 2.0f },
                { " 30 fps (4 步/帧)  ", Spaceship::FIXED_DT * 4.0f },
            };
            printf("[T4] AdvanceFixed 不变式（每档 600 帧；alpha 必须 <1、残差必须 <FIXED_DT）\n");
            bool t4 = true;
            for (const Case& c : cases) {
                Spaceship s; float accum = 0.0f, alpha = 0.0f;
                int total = 0, mn = 1 << 30, mx = 0;
                float mxAlpha = 0.0f, mxRes = 0.0f;
                for (int f = 0; f < 600; ++f) {
                    const int n = s.AdvanceFixed(accum, c.dt, alpha);
                    total += n; mn = glm::min(mn, n); mx = glm::max(mx, n);
                    mxAlpha = glm::max(mxAlpha, alpha);
                    mxRes = glm::max(mxRes, accum);
                }
                const float expect = 600.0f * c.dt / Spaceship::FIXED_DT;
                const bool ok = (mxAlpha < 1.0f) && (mxRes < Spaceship::FIXED_DT)
                    && (glm::abs(float(total) - expect) <= 1.0f);
                t4 = t4 && ok;
                printf("     %s 步/帧 %d~%d 共 %4d（理论 %.1f）| alpha max %.6f | 残差 max %.6f s | %s\n",
                    c.name, mn, mx, total, expect, mxAlpha, mxRes, ok ? "OK" : "FAIL");
            }
            printf("     >>> T4: %s\n\n", t4 ? "PASS" : "FAIL");
        }
        printf("===================================================\n\n");
    }

#else   // !DEBUG_PHYS —— Release 下退化成空函数，调用点不用 #ifdef

    inline void Run() {}

#endif
}