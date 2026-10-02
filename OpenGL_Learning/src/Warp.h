#pragma once
// ---------------------------------------------------------------------------
//  Warp.h —— 战术折跃的时间线。纯函数：无状态、不含 GL、不含 glm。
//
//  本文件里的所有量都是唯一时钟 tau 的函数，tau 是自按下 T 键起算的秒数。
//  这里不碰物理步进器：折跃是视觉事件，它唯一落在物理侧的动作是
//  Spaceship::WarpTo 那一次传送（计划书 §5.2）。
//
//  保持不含 glm 是有意的：阶段 1 的自测要能直接 include 本文件而不拉起
//  整个渲染依赖。
// ---------------------------------------------------------------------------

#include <cmath>

namespace warp
{
    // ===== 四段标称时长（秒）===============================================
    //  蓄能 [0.00, 0.45)  电流外壳由弱到强
    //  隐没 [0.45, 0.65)  舰体白化并拉伸，段末闪现达峰
    //  线框 [0.65, 1.00)  线条舰体自艏向艉生长
    //  实体化 [1.00, 1.25) 材质由白热恢复
    //  注意：五个通道全都在 tau = 1.22 之前归零，最后的 0.03 s 是余量，
    //  用来容纳"钉住一帧"造成的时间线延长（见下方 advance）。
    inline constexpr float T_CHARGE = 0.90f;    // 0.45
    inline constexpr float T_VANISH = 0.20f;    // 0.20
    inline constexpr float T_WIRE   = 0.70f;    // 0.35
    inline constexpr float T_APPEAR = 0.25f;    // 0.25
    inline constexpr float T_TOTAL = T_CHARGE + T_VANISH + T_WIRE + T_APPEAR;

    // 传送时刻 = 闪现通道的峰值时刻。舰体的传送与相机的位移必须发生在这
    // 一帧，且只能发生在这里（计划书 §4.3：只有在闪现权重为 1 时传送，
    // 背景的视差跳变才会被完全遮住）。
    inline constexpr float TAU_STAR = T_CHARGE + T_VANISH;      // 0.65
    // 实体化开始时刻 = 线框段结束。它同时是"几何退回"与"拉伸开始回落"的起点，
    // 所以和 TAU_STAR 并列声明：全文所有"绝对秒数"都改写成相对这两个时刻的偏移。
    inline constexpr float T_WIRE_END = T_CHARGE + T_VANISH + T_WIRE;   // 1.00

    // 闪现通道：上升起点、峰值（等于 TAU_STAR）、衰减终点、衰减幂次。
    // 上升段 0.05 s，衰减段 0.09 s，刻意不对称。
    /*inline constexpr float FLASH_RISE = 0.60f;*/
    inline constexpr float FLASH_RISE = TAU_STAR - 0.05f;
    /*inline constexpr float FLASH_END = 0.74f;*/
    inline constexpr float FLASH_END = TAU_STAR + 0.09f;

    inline constexpr float FLASH_POW  = 2.2f;

    // 单帧时钟增量的上界（计划书 §5.4）。deltaTime 取自 glfwGetTime 的裸
    // 差值，一次卡顿就能让 tau 跨过整段，把闪现整个跳过去；钳到 1/30 s
    // 保证任何通道都不会被跳过一整个上升沿。
    inline constexpr float DT_MAX = 1.0f / 30.0f;

    // 慢放开关（计划书 §10.3）。1.0 为实时；置 0.1 则整段折跃放慢 10 倍，
    // 用来逐帧确认遮盖与生长过程。因为一切由 tau 驱动，慢放不改变任何
    // 几何关系，只是把同一条参数化曲线拉长。
    inline constexpr float TIME_SCALE = 1.0f;

    // ===== 五个通道的梯形参数（计划书 §3.2）===============================
    //        r0     r1     f0     f1
    //  u_c   0.00   0.40   0.45   0.62   电流
    //  u_v   0.45   0.60   0.62   0.68   白化与拉伸
    //  u_w   0.62   0.70   0.95   1.08   线框
    //  u_a   0.98   1.05   1.10   1.22   实体化
    //  u_g 不用梯形，见下方 flash（它需要单侧尖点）。
    //
    //  每一行必须满足 r1 <= f0，否则上升段与下降段在 (f0, r1) 上相乘，
    //  峰值达不到 1 且不会报错（计划书 注 1）。通道之间的"重叠"是靠相邻
    //  通道的区间搭接实现的，不要在某一行内部制造交叠。
    //inline constexpr float C_R0 = 0.00f, C_R1 = 0.40f, C_F0 = 0.45f, C_F1 = 0.62f;
    //inline constexpr float V_R0 = 0.45f, V_R1 = 0.60f, V_F0 = 0.62f, V_F1 = 0.68f;
    //inline constexpr float W_R0 = 0.62f, W_R1 = 0.70f, W_F0 = 0.95f, W_F1 = 1.08f;
    //inline constexpr float A_R0 = 0.98f, A_R1 = 1.05f, A_F0 = 1.10f, A_F1 = 1.22f;
        //  上表是 T_CHARGE=0.45 / T_VANISH=0.20 / T_WIRE=0.35 / T_APPEAR=0.25 时的取值，
    //  只作对照。代码一律写成相对量，于是"每行 r1 <= f0"与"交棒对齐"都由构造保证：
    //  交棒点 = 前一个通道开始淡出的时刻 = 后一个通道开始上升的时刻；
    //  0.03 是给交棒留的让位量（线框要赶在闪现峰值前先起一点，材质要赶在实体化前先起一点）。
    inline constexpr float 
        C_R0 = 0.00f, 
        C_R1 = T_CHARGE - 0.05f, 
        C_F0 = T_CHARGE, 
        C_F1 = TAU_STAR - 0.03f;
    inline constexpr float 
        V_R0 = C_F0,     
        V_R1 = FLASH_RISE, 
        V_F0 = C_F1, 
        V_F1 = TAU_STAR + 0.03f;
    inline constexpr float 
        W_R0 = V_F0, 
        W_R1 = TAU_STAR + 0.05f, 
        W_F0 = T_WIRE_END - 0.05f, 
        W_F1 = T_WIRE_END + 0.08f;
    inline constexpr float 
        A_R0 = T_WIRE_END - 0.02f, 
        A_R1 = T_WIRE_END + 0.05f, 
        A_F0 = T_WIRE_END + 0.10f, 
        A_F1 = T_TOTAL - 0.03f;

    // ===== 观感参数（计划书 §2、§11.1）===================================
    inline constexpr float E_STRETCH = 0.35f;    // 沿局部 -Y 的拉伸系数上界
    inline constexpr float KAPPA_BACK = 0.015f;   // 蓄能期反向退距比（退 kappa*D）

    // ===== 缓动函数（计划书 §3.3）=========================================
    inline float clamp01(float x)
    {
        return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
    }

    // smoothstep：两端一阶导为零（C1），但两端二阶导为 ±6，不是 C2。
    // 只用于强度类量：亮度的高阶导数不可见，C1 足够。
    inline float smoothstep01(float x)
    {
        x = clamp01(x);
        return x * x * (3.0f - 2.0f * x);
    }

    // 五次缓动 q = 6x^5 - 15x^4 + 10x^3：两端一阶导与二阶导都为零（C2）。
    // 用于位移、缩放、FOV 回位：位移的二阶导是加速度，起点处加速度突变
    // 在观感上就是"起步的那一顿"。
    inline float quintic01(float x)
    {
        x = clamp01(x);
        return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
    }

    // ===== 梯形权重 ========================================================
    //  trap(tau) = 上升因子 * (1 - 下降因子)，峰值等于 1 当且仅当 r1 <= f0。
    inline float trap(float tau, float r0, float r1, float f0, float f1)
    {
        const float up   = smoothstep01((tau - r0) / (r1 - r0));
        const float down = smoothstep01((tau - f0) / (f1 - f0));
        return up * (1.0f - down);
    }

    // ===== 闪现通道：不对称，在 TAU_STAR 处有单侧尖点 ======================
    //  上升段是线性的，斜率 +20 /s；下降段是幂律，峰值处斜率 -24.4 /s。
    //  导数在这里刻意不连续（不是 C1）：尖点就是"亮度瞬间到顶、随即开始
    //  衰减"的数学表述。把它磨成 C1（例如上升段改用 smoothstep）会在峰值
    //  前造出一段斜率趋零的平台，闪现就变成了渐变。不要"修正"它。
    //  另外 p > 1 使衰减斜率的绝对值单调递减，即"先快后慢"。
    inline float flash(float tau)
    {
        if (tau <= TAU_STAR)
            return clamp01((tau - FLASH_RISE) / (TAU_STAR - FLASH_RISE));

        const float x = (tau - TAU_STAR) / (FLASH_END - TAU_STAR);
        if (x >= 1.0f) return 0.0f;
        return std::pow(1.0f - x, FLASH_POW);
    }

    // ===== 五个通道的权重 ==================================================
    struct Channels
    {
        float c = 0.0f;   // 电流外壳
        float v = 0.0f;   // 白化与拉伸
        float g = 0.0f;   // 全屏闪现
        float w = 0.0f;   // 线框
        float a = 0.0f;   // 材质恢复
        float solid = 1.0f;   // 舰体实体度，决定投不投影（计划书 §5.3）
    };

    inline Channels sample(float tau)
    {
        Channels k;
        k.c = trap(tau, C_R0, C_R1, C_F0, C_F1);
        k.v = trap(tau, V_R0, V_R1, V_F0, V_F1);
        k.g = flash(tau);
        k.w = trap(tau, W_R0, W_R1, W_F0, W_F1);
        k.a = trap(tau, A_R0, A_R1, A_F0, A_F1);

        // 实体度：白化过半或线框过半就停止投影。深度 pass 无法做半透明，
        // 所以这是一个二值判据，门限取 0.5。
        k.solid = clamp01(1.0f - k.v - k.w);
        return k;
    }

    // ===== 预备动作与拉伸 ==================================================
    //  归一化退距，取值为 [0,1]：在蓄能段内升起，在 TAU_STAR 处精确回到 0。
    //  调用方乘以 KAPPA_BACK * D 并沿 -f_hat 施加到【模型矩阵】上，绝不施加
    //  到 ship.position —— 它是纯视觉偏移，一旦进入物理就会破坏传送的
    //  位移不变式。
    inline float anticipation(float tau)
    {
        if (tau <= 0.0f) return 0.0f;
        if (tau <= T_CHARGE) return quintic01(tau / T_CHARGE);
        if (tau <= TAU_STAR)return 1.0f - quintic01((tau - T_CHARGE) / T_VANISH);
        return 0.0f;
    }

    ////  沿舰体局部 -Y 轴的拉伸系数。用五次曲线而非线性，是为了让拉伸的
    ////  加速度也连续（计划书 §3.3 的分配原则）。
    //inline float stretchFactor(float vWeight)
    //{
    //    return 1.0f + E_STRETCH * quintic01(vWeight);
    //}
    
    // ===== 拉伸：去程升起、抵达收缩（计划书 §8 阶段 3）===================
    //  为什么不让它跟着 u_v 走：u_v 是隐没段的白化通道，抵达段已经是 0，
    //  于是"抵达时拉长着出现、再缩回原比例"这个动作根本不会发生。
    //  做成一条独立的包络之后，峰值落在 τ*（闪现那一帧），两侧都连续 ——
    //  传送正好被闪现盖住，所以整条曲线读起来是一次完整的"拉伸到还原"。
    //inline constexpr float STRETCH_RISE = 0.50f;   // 拉伸起点（隐没段之内）
    //inline constexpr float STRETCH_FALL = 0.55f;   // 自 τ* 收缩到 1 所用的时间

    ////  返回"拉伸量" a ∈ [0,1]，缓动已经在这里做完。
    //inline float stretchAmount(float tau)
    //{
    //    if (tau <= STRETCH_RISE) return 0.0f;
    //    if (tau <= TAU_STAR)     return quintic01((tau - STRETCH_RISE) / (TAU_STAR - STRETCH_RISE));
    //    if (tau >= TAU_STAR + STRETCH_FALL) return 0.0f;
    //    return 1.0f - quintic01((tau - TAU_STAR) / STRETCH_FALL);
    //}

    inline constexpr float STRETCH_RISE = TAU_STAR - 0.15f;   // 拉伸起点（隐没段之内）
    inline constexpr float STRETCH_HOLD_END = T_WIRE_END;     // 1.00 = T_WIRE_END
    inline constexpr float STRETCH_FALL = T_APPEAR;                          // 0.25，与实体化同期收完
    
    //  返回"拉伸量" a ∈ [0,1]，缓动已经在这里做完。
    inline float stretchAmount(float tau)
    {
        if (tau <= STRETCH_RISE)     return 0.0f;
        if (tau <= TAU_STAR)         return quintic01((tau - STRETCH_RISE) / (TAU_STAR - STRETCH_RISE));
        if (tau <= STRETCH_HOLD_END) return 1.0f;
        if (tau >= STRETCH_HOLD_END + STRETCH_FALL) return 0.0f;
        return 1.0f - quintic01((tau - STRETCH_HOLD_END) / STRETCH_FALL);
    }

    //  拉伸系数 = 1 + E_STRETCH * a。缓动已由 stretchAmount 完成，这里只做线性映射。
    inline float stretchFactor(float amount)
    {
        return 1.0f + E_STRETCH * amount;
    }

    // ===== 时钟推进，含 TAU_STAR 的"钉住一帧"（计划书 §4.4）===============
    //  钉住是必需的：闪现上升段只有 0.05 s，30 fps 下时钟每帧就走 1/30 s，
    //  峰值可能被整帧跨过去（tau 从 0.59 跳到 0.623 再到 0.657，闪现权重
    //  依次为 0、0.466、0.842，从未取到 1），传送就会发生在没被遮住的帧上。
    //  做法是：跨越 TAU_STAR 的那一帧把 tau 钳到 TAU_STAR 恰好执行，下一帧
    //  再从 TAU_STAR 继续推进。代价是时间线最多延长一帧，而那一帧恰好是
    //  全屏最亮的一帧，看不见。
    struct Step
    {
        float tau = 0.0f;
        bool  teleport = false;   // 本帧执行传送，且仅本帧
        bool  finished = false;   // 时间线已越过 T_TOTAL
    };

    inline Step advance(float& tau, float frameDt)
    {
        Step s;

        float dt = frameDt < 0.0f ? 0.0f : frameDt;
        if (dt > DT_MAX) dt = DT_MAX;
        dt *= TIME_SCALE;

        if (tau < TAU_STAR && tau + dt >= TAU_STAR)
        {
            tau = TAU_STAR;          // 本帧精确钉在峰值上
            s.tau = tau;
            s.teleport = true;
            return s;
        }

        tau += dt;
        s.tau = tau;
        s.finished = (tau >= T_TOTAL);
        return s;
    }
    // ===== 溶解前沿（计划书 §3.5）=========================================
    //  两条同终点、不同起点的斜坡，都在 TAU_STAR 走完；线框段保持满值；
    //  实体化段按同一条 σ 退回 0。返回值都定义在轴向坐标 g 上（0 在艏、1 在艉）。
    //
    //  为什么不用"一条前沿减去固定滞后"：那样几何前沿最多到 1 - 滞后，闪现那一帧
    //  艉部会残下约 18% 的裸结构，而且线框段它还会跟着画到目的位置。
    //inline constexpr float T_WIRE_END = T_CHARGE + T_VANISH + T_WIRE;
    
    
    inline constexpr float NOISE_W = 0.25f;

    //inline constexpr float FRONT_MAT_START = 0.47f;           // 材质剥离前沿起点
    inline constexpr float FRONT_GEO_START = TAU_STAR - 0.12f;  // 几何剔除前沿起点
    inline constexpr float FRONT_RETREAT = 0.8f * T_APPEAR;     // 返程退回时长 = 0.20
    //inline constexpr float FRONT_OVER = 1.02f;                // 越冲量，理由见 geoFront
    
    //  前沿的取值域要覆盖噪声的取值范围，而不是 [0,1]：
    //  gt = g + NOISE_W*(n - 0.5)，其中 n ∈ [0,1]、g ∈ [0,1]，所以 gt ∈ [-w/2, 1 + w/2]。
    //  取满这个范围之后"静止"与"完成"才是精确的端点语义：
    //    静止（r = 0）：几何前沿低于噪声下界，剔除条件恒不成立；
    //    完成（r = 1）：几何前沿高于噪声上界，剔除条件恒成立。
    inline constexpr float FRONT_LO   = -0.5f * NOISE_W;          // -0.125
    inline constexpr float FRONT_HI   = 1.0f + 0.5f * NOISE_W;    //  1.125
    //inline constexpr float FRONT_LEAD = 0.06f;                    // 材质领先几何的空间量
    inline constexpr float FRONT_LEAD = 0.25f;                    // 材质领先几何的空间量

    //  在 [start, TAU_STAR] 上由 0 升到 1。两端都落在 σ 的端点，而 σ'(0) = σ'(1) = 0，
    //  所以整条曲线是 C1 的，侵蚀速率不会突然起跳。调用方须保证 start < TAU_STAR。
    inline float frontRamp(float tau, float start)
    {
        if (tau <= start)       return 0.0f;
        if (tau >= TAU_STAR)    return 1.0f;

        return smoothstep01((tau - start) / (TAU_STAR - start));
    }


    //  几何剔除前沿。r 是进度：去程由 0 升到 1（TAU_STAR 走完），线框段保持 1，
    //  返程退回 0。r = 0 时它落在 FRONT_LO - FRONT_LEAD，比噪声下界还低；
    //  r = 1 时落在 FRONT_HI，比噪声上界还高 —— 两端都精确。
    inline float geoFront(float tau)
    {
        float r;
        if (tau <= TAU_STAR)        r = frontRamp(tau, FRONT_GEO_START);
        else if (tau <= T_WIRE_END) r = 1.0f;                    // 线框段：全剥完
        else                        r = 1.0f - smoothstep01((tau - T_WIRE_END) / FRONT_RETREAT);

        return (FRONT_LO - FRONT_LEAD) + (FRONT_HI - FRONT_LO + FRONT_LEAD) * r;
    }

    //  材质剥离前沿 = 几何前沿 + 固定领先量。两者的间距恒为 FRONT_LEAD > 0，
    //  所以 smoothstep 的两个边界永不相等、也永不交叉 —— 这同时消掉了上一版那个
    //  FRONT_OVER 越冲常数，以及它造成的边界反转。
    inline float matFront(float tau) { return geoFront(tau) + FRONT_LEAD; }


    // ===== 阶段 4：FOV 冲击与短促抖动（计划书 §8 阶段 4）==================
    //  两者都起于 τ* 之前、峰值恰好落在 τ*，并在 T_WIRE_END 之前归零。
    //  峰值放在 τ* 的理由是**峰值处导数为零**：两条包络在 τ* 的左右导数都是 0，
    //  所以那一帧它们对相机偏移与 FOV 的帧间增量恰好为 0，命题 2 要求的
    //  "舰体与相机平移同一个向量"按帧间增量精确成立；峰值若错开 τ*，那一帧
    //  相机就会多出一个非零增量，舰体的屏幕姿态会跟着挪一下。
    //  在 T_WIRE_END 之前归零是为了不污染后续的相机阻尼（跟随相机每帧朝目标
    //  mix，残留偏移会一直带着走）。。
    inline constexpr float FOV_KICK    = 3.5f;                  // Φ，单位是度
    inline constexpr float SHAKE_AMT   = 0.30f;                 // 世界单位
    inline constexpr float KICK_START  = TAU_STAR - 0.07f;      // FOV 冲击起点
    inline constexpr float KICK_END    = TAU_STAR + 0.30f;      // FOV 冲击归零点
    inline constexpr float SHAKE_START = TAU_STAR - 0.02f;      // 抖动起点（在闪现上升段之内）
    inline constexpr float SHAKE_DECAY = 0.15f;                 // 抖动衰减时长

    //  FOV 冲击是"长包络"：起得早、落得慢，读作一次推镜。
    //  两段都用五次曲线，两端 φ'(0) = φ'(1) = φ''(0) = φ''(1) = 0，
    //  所以整条曲线 C2 —— 起止的**加速度**也连续，不会有"一顿"。
    inline float fovKick(float tau)
    {
        if (tau <= KICK_START) return 0.0f;
        if (tau <= TAU_STAR)   return quintic01((tau - KICK_START) / (TAU_STAR - KICK_START));
        if (tau >= KICK_END)   return 0.0f;
        return 1.0f - quintic01((tau - TAU_STAR) / (KICK_END - TAU_STAR));
    }

    //  抖动是"短包络"：只跨闪现那一瞬间，之后 0.15 s 内落回 0。
    //  刻意不复用 fovKick —— 那样抖动会拖到 0.37 s，变成缓慢漂移而不是"短促一顶"。
    inline float shakeEnvelope(float tau)
    {
        if (tau <= SHAKE_START) return 0.0f;
        if (tau <= TAU_STAR)    return quintic01((tau - SHAKE_START) / (TAU_STAR - SHAKE_START));
        if (tau >= TAU_STAR + SHAKE_DECAY) return 0.0f;
        return 1.0f - quintic01((tau - TAU_STAR) / SHAKE_DECAY);
    }


    // ===== 阶段 5：线框的生长位置（计划书 §8 阶段 5）=====================
    //  u_w 是梯形，在 [0.95, 1.08] 上会落下 —— 不能拿它当生长位置，否则线框会
    //  从艉向艏缩回去。所以把"位置"和"强度"分成两件事：
    //    位置 = 下面这个单调函数（只前进）
    //    强度 = u_w（整体明暗，含末段被实体取代时的淡出）
    //inline constexpr float WIRE_GROW_TIME = 0.20f;   // 自艏扫到艉所用的时间

    //inline float wireGrow(float tau)
    //{
    //    if (tau <= TAU_STAR) return 0.0f;
    //    if (tau >= TAU_STAR + WIRE_GROW_TIME) return 1.0f;
    //    return smoothstep01((tau - TAU_STAR) / WIRE_GROW_TIME);
    //}

    // ===== 阶段 5：线框的生长位置（计划书 §8 阶段 5）=====================
    //  生长位置 = 下面这根单调曲线（只前进）；强度 = wireAlpha（见下）。
    //  曲线自按键（τ = 0）起画，WIRE_GROW_TIME 秒画满落点的骨架，之后恒为完整 ——
    //  也就是说"自艏向艉"的揭示发生在**蓄能期**，传送之后不再生长。
   
    //  为什么传送后不能再生长：传送后那份线框与蓝图是同一世界位置上的同一副骨架，
    //  前沿若还没到头，闪白之后它会把蓝图画了一半的截"接手"过去，读起来是蓝图白画。
    //  材质重建（geoFront 的退回）在 [1.00, 1.20] 仍会自艏向艉揭示一次，不会丢。
    //inline constexpr float WIRE_GROW_TIME = T_CHARGE  // 自艏画到艉所用的时间 = 整个蓄能段
    inline constexpr float WIRE_GROW_TIME = TAU_STAR;   //   TAU_STAR  = T_CHARGE + T_VANISH

    inline float wireGrow(float tau)
    {
        if (tau <= 0.0f)           return 0.0f;
        if (tau >= WIRE_GROW_TIME) return 1.0f;
        return smoothstep01(tau / WIRE_GROW_TIME);  // 平滑过渡版本
        //return tau / WIRE_GROW_TIME;  // 线性过渡版本

        //const float x = tau / WIRE_GROW_TIME;
        //return 1.0f - (1.0f - x) * (1.0f - x);  // 起步快收尾慢的过渡版本
    }

    // ===== 线框（含蓄能期的蓝图）的强度 ==================================
    //  只保留 u_w 的【下降沿】：蓄能期就全亮、τ* 处不跳变、末段随材质取代而淡出。
    //  为什么不直接用 u_w：它的上升段 [0.62, 0.70] 是为"线框随传送出现"设计的；
    //  现在落点的骨架自蓄能期就在场，再淡入一次就会在 τ* 前后掉一次亮度 ——
    //  模式 1/2 没有全屏白遮着，那一掉就是一次闪断。
    //  下降沿仍引用 W_F0 / W_F1，所以淡出时机照样只在通道表里改一处。
    inline float wireAlpha(float tau)
    {
        if (tau <= 0.0f) return 0.0f;
        return 1.0f - smoothstep01((tau - W_F0) / (W_F1 - W_F0));
    }

}



