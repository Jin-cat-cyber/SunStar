#pragma once
// ---------------------------------------------------------------------------
//  WarpSC.h —— SC2 风格折跃的时间线（Warp.h 的分支副本，命名空间 warp_sc）。
//
//  【本文件是分支】与 Warp.h 的差异一律在本文件内用 "SC 分支：" 标注，
//  方便与原版逐行对照。第一步是逐字节副本，行为应与原版完全一致。
//
//  本文件里的所有量都是唯一时钟 tau 的函数，tau 是自按下 T 键起算的秒数。
//  这里不碰物理步进器：折跃是视觉事件，它唯一落在物理侧的动作是
//  Spaceship::WarpTo 那一次传送（计划书 §5.2）。
//
//  保持不含 glm 是有意的：阶段 1 的自测要能直接 include 本文件而不拉起
//  整个渲染依赖。
// ---------------------------------------------------------------------------

#include <cmath>

namespace warp_sc
{
    // ===== 四段标称时长（秒）===============================================
    //  蓄能 [0.00, 0.90)  电流外壳由弱到强
    //  隐没 [0.90, 1.10)  舰体白化并拉伸，段末闪现达峰
    //  线框 [1.10, 1.80)  线条舰体自艏向艉生长
    //  实体化 [1.80, 2.05) 材质由白热恢复
    //  注意：五个通道全都在 T_TOTAL - 0.03 之前归零，那 0.03 s 是余量，
    //  用来容纳"钉住一帧"造成的时间线延长（见下方 advance）。
    inline constexpr float T_CHARGE = 1.35f;    //0.90f;    // 0.45f;
    inline constexpr float T_VANISH = 0.20f;    // 0.20
    //inline constexpr float T_WIRE = 0.70f;    // 0.35
    
    //  骨架段 = 生长 + 静止等待 + 光柱比材质早出现的那一小段。
    //  * 想改"骨架长满后等多久"，只动 HOLD_TIME 这一个数，其余全是派生量。
    inline constexpr float GROW_SPAN = 0.47f;      // 骨架自艉向艏生长时长。
    inline constexpr float HOLD_TIME = 2.87f;      // 骨架长满后画面静止的等待时长。
    inline constexpr float PILLAR_LEAD = 0.10f;    // 光柱比材质早出现的时长。
    inline constexpr float T_WIRE = GROW_SPAN + HOLD_TIME + PILLAR_LEAD;
    
    //inline constexpr float T_APPEAR = 0.25f;    // 0.25
    //inline constexpr float T_APPEAR = 0.40f;    // = FILL_TIME + WIRE_TAIL（实体化段全长）
    //inline constexpr float T_APPEAR = 0.53f;    // = FILL_TIME(0.30) + WIRE_TAIL(0.23)（实体化段全长）
    inline constexpr float T_APPEAR = 0.63f;    // = FILL_TIME(0.30) + WIRE_TAIL(0.33)（实体化段全长）

    //  实体化段拆成两个子窗口（骨架余韵必须晚于材质填充结束）：
    //    [0, FILL_TIME)          材质填充：前沿反跑 + 整体凝实 + 滑入 + 拉伸收回
    //    [FILL_TIME, T_APPEAR]   骨架余韵：保持全亮，再用 WIRE_TAIL 淡出
    inline constexpr float FILL_TIME = 0.30f;   // 材质填充时长
    inline constexpr float WIRE_TAIL = T_APPEAR - FILL_TIME;   // 填充完成后骨架多留的时间（0.10）
    
    inline constexpr float T_TOTAL = T_CHARGE + T_VANISH + T_WIRE + T_APPEAR;


    // ===== SC 分支：把"波扫到哪"翻译成时刻 ==================================
    //  去程扫描从 FRONT_GEO_START 走到 TAU_STAR，时长就是下面这个 SCAN_TIME。
    //  scanTau(p) 收的是【归一化时间】p ∈ [0,1]，不是前沿位置 r —— 两者由 r = σ(p) 联系，
    //  所以"p = 0.33"对应"前沿刚越过艏"、"p = 0.75"对应"前沿到舰尾附近"、"p = 1"对应"剥完"
    //  （对照表在精读第 7 段）。把前冲、拉伸、外壳这些包的时间点写成 scanTau(p) 之后，
    //  "波到哪一步才开始动"在代码里就是字面值，改四段时长也不会把相位带偏。
    inline constexpr float SCAN_TIME = 0.60f;                 // 去程扫描时长
    

    // 传送时刻 = 闪现通道的峰值时刻。舰体的传送与相机的位移必须发生在这
    // 一帧，且只能发生在这里（计划书 §4.3：只有在闪现权重为 1 时传送，
    // 背景的视差跳变才会被完全遮住）。
    inline constexpr float TAU_STAR = T_CHARGE + T_VANISH;

    //  scanTau 必须放在 TAU_STAR 之后：函数体里要用 TAU_STAR，而常量表达式里的名字
    //  按定义点查找 —— 放前面编译器会说"表达式的计算结果不是常数"（报在使用点，不在定义点）。
    //  它同时给常量（SLIDE_RISE / STRETCH_RISE）与运行期（shellAlpha）用，所以声明成 constexpr。
    inline constexpr float scanTau(float p) { return (TAU_STAR - SCAN_TIME) + p * SCAN_TIME; }

    // 实体化开始时刻 = 线框段结束。它同时是"几何退回"与"拉伸开始回落"的起点，
    // 所以和 TAU_STAR 并列声明：全文所有"绝对秒数"都改写成相对这两个时刻的偏移。
    inline constexpr float T_WIRE_END = T_CHARGE + T_VANISH + T_WIRE;

    // 闪现通道：上升起点、峰值（等于 TAU_STAR）、衰减终点、衰减幂次。
    // 上升段 0.05 s，衰减段 0.09 s，刻意不对称。
    /*inline constexpr float FLASH_RISE = 0.60f;*/
    inline constexpr float FLASH_RISE = TAU_STAR - 0.05f;
    /*inline constexpr float FLASH_END = 0.74f;*/
    inline constexpr float FLASH_END = TAU_STAR + 0.09f;

    inline constexpr float FLASH_POW = 2.2f;

    // 单帧时钟增量的上界（计划书 §5.4）。deltaTime 取自 glfwGetTime 的裸
    // 差值，一次卡顿就能让 tau 跨过整段，把闪现整个跳过去；钳到 1/30 s
    // 保证任何通道都不会被跳过一整个上升沿。
    inline constexpr float DT_MAX = 1.0f / 30.0f;

    // 慢放开关（计划书 §10.3）。1.0 为实时；置 0.1 则整段折跃放慢 10 倍，
    // 用来逐帧确认遮盖与生长过程。因为一切由 tau 驱动，慢放不改变任何
    // 几何关系，只是把同一条参数化曲线拉长。
    inline constexpr float TIME_SCALE = 1.0f;   // 1.0 = 真实速度；看片时可临时设小值慢放，交版前必须改回 1.0。

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
        W_F0 = T_WIRE_END + FILL_TIME,   // 材质填充完成，骨架才开始淡出
        W_F1 = T_WIRE_END + T_APPEAR;    // 骨架余韵淡完（= T_TOTAL）
    inline constexpr float
        A_R0 = T_WIRE_END - 0.02f,
        A_R1 = T_WIRE_END + 0.05f,
        A_F0 = T_WIRE_END + 0.10f,
        A_F1 = T_TOTAL - 0.03f;

    // ===== 观感参数（计划书 §2、§11.1）===================================
    //inline constexpr float E_STRETCH = 0.35f;    // 沿局部 -Y 的拉伸系数上界
    inline constexpr float E_STRETCH = 0.35f;    // 沿局部 -Y 的拉伸系数上界（峰值拉伸比 = 1 + E）
    inline constexpr float PINCH_K = 2.00f;    // 横向挤压倍率：pinch = 1 + (拉伸比 - 1) * K
    
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

    // 二次 ease-out：起步最快、到点速度归零。
    // 给抵达段的滑入用 —— 参考画面里舰体冒出来时就带着速度、一路减速到落点停住。
    // 五次缓动起点速度为零，滑入走到 1/3 时间点时只走了 21% 行程；而参考那一刻
    // 舰体已经走过约六成（进框约 44%），差距全在起步这一段。
    // 起点速度不为零是有意的：4.99 那一刻画面上只有舰首刚露出来的一条，突变看不见；
    // 而且原版本来就是"飞进来"的。
    inline float easeOutQuad(float x)
    {
        x = clamp01(x);
        const float u = 1.0f - x;
        return 1.0f - u * u;
    }


    // ===== 梯形权重 ========================================================
    //  trap(tau) = 上升因子 * (1 - 下降因子)，峰值等于 1 当且仅当 r1 <= f0。
    inline float trap(float tau, float r0, float r1, float f0, float f1)
    {
        const float up = smoothstep01((tau - r0) / (r1 - r0));
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

    //inline constexpr float STRETCH_RISE = TAU_STAR - 0.15f;   // 拉伸起点（隐没段之内）
    //inline constexpr float STRETCH_RISE_P = 0.75f;             // 拉伸起点：扫描进度（≈ 前沿到舰尾附近）
    inline constexpr float STRETCH_RISE_P = 0.444f;            // 拉伸起点：半透明波扫到中垂面（g = 0.5）的进度
    inline constexpr float STRETCH_RISE = scanTau(STRETCH_RISE_P);

    //inline constexpr float STRETCH_FALL = FILL_TIME;          // 收回时长（与材质填充同期）
        
    //  收回延后一点：材质整段是落在骨架【之尾】的，要等舰首伸进骨架范围才开始复原。
    //  延后多少，时长就缩短多少 —— 这样才能与滑入【同时结束】。
    inline constexpr float STRETCH_DELAY = 0.10f;                       // 收回的延后量
    inline constexpr float STRETCH_FALL = FILL_TIME - STRETCH_DELAY;    // 收回时长 = 0.20
    //  保持段延伸到填充开始之后：填充是"自艏向艉"逐段出现的，保持段若在填充开始时就结束，
    //  舰体一露面拉伸已经在收，等于"拉长着出现"根本看不到。
        
    //  拉伸的"保持"在材质开始填充那一刻就结束 —— 与滑入同一时刻开始、同一时长、
    //  同一时刻完成。于是"一边从压缩状态复原、一边滑入，归位时就完全归位"。
    //  不要再用 T_APPEAR - STRETCH_FALL 去凑"用满实体化段"：那条式子会把收回的起点
    //  跟着 T_APPEAR（线框尾巴的长度）一起往后拖，结果是滑入先完成、舰体归位之后
    //  还在慢慢复原。这两件事必须解耦。
    //inline constexpr float STRETCH_HOLD_EXT = 0.0f;
    inline constexpr float STRETCH_HOLD_EXT = STRETCH_DELAY;
    inline constexpr float STRETCH_HOLD_END = T_WIRE_END + STRETCH_HOLD_EXT;    // = 1.90

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


    // ===== SC 分支：前向运动与滑入 ========================================
    //  返回带符号的"行程比例" s ∈ [-1, +1]；调用方乘以舰体半个长度
    //  （cpp 里的 gShipBowOffset），再沿 RenderForward 前乘到模型矩阵上。
    //
    //    τ < τ*：s 由 0 升到 +1  —— 去程向前冲出，读作"穿过一个固定的竖直面"；
    //    τ ≥ τ*：s 由 -1 回到 0  —— 抵达时几何先在落点前一个行程处出现，
    //                               再滑入线框，同时完成复位。
    //
    //  为什么只进模型矩阵：它是纯视觉量，写进 ship.position 就会破坏传送的位移
    //  不变式（P4/P5）；而且相机不跟它走，背景就不会跳，白闪仍然只需要遮 τ*。
    //  τ* 处的翻号看不见：那一帧舰体已被完全剥除（geoFront = FRONT_HI）。
    //inline constexpr float SLIDE_RISE = TAU_STAR - 0.40f;      // 去程起冲时刻

    //  起冲时刻改用【扫描进度】表达，p = 0.33 约等于"前沿刚越过艏"（见 scanTau 的注释）。
    inline constexpr float SLIDE_RISE_P = 0.33f;               // 起冲：扫描进度
    inline constexpr float SLIDE_RISE = scanTau(SLIDE_RISE_P);

    inline constexpr float SLIDE_HOLD_END = T_WIRE_END;        // 抵达段开始滑入的时刻
    //inline constexpr float SLIDE_FALL = T_APPEAR;              // 抵达滑入时长
    //inline constexpr float SLIDE_FALL = FILL_TIME;             // 抵达滑入时长（与填充同期）
    inline constexpr float SLIDE_FALL = FILL_TIME + 0.15f;     // 抵达滑入时长：比还原多 0.15 s，末段是纯滑动（对照 SC2 15s05~09 帧）

    inline float slideAmount(float tau)
    {
        if (tau <= SLIDE_RISE) return 0.0f;
        if (tau <= TAU_STAR)   return quintic01((tau - SLIDE_RISE) / (TAU_STAR - SLIDE_RISE));
        // SC 分支：抵达段先在逻辑面上钉住（-1），等到实体化段再滑回 0。
        // 理由：舰体在骨架段被整段剔除（geoFront = FRONT_HI），滑入若在骨架段里跑完就完全看不见。
        // 钉在 -1 期间也不影响别的 pass：骨架段舰体既不画也不投影（solidNow 为负）。
        if (tau <= SLIDE_HOLD_END) return -1.0f;
        if (tau >= SLIDE_HOLD_END + SLIDE_FALL) return 0.0f;
        //return -1.0f + quintic01((tau - SLIDE_HOLD_END) / SLIDE_FALL);
        return -1.0f + easeOutQuad((tau - SLIDE_HOLD_END) / SLIDE_FALL);
    }


    // ===== SC 分支：两段行程的倍率 ==========================================
    //  一个 slideAmount 单位 = gShipBowOffset（半个舰长），两段各乘一个倍率：
    //    去程 SLIDE_DEPART_K：冲出半个舰长 × K。前冲量直接决定落点合法性余量 ——
    //      cpp 那边的余量里必须带上 K * gShipBowOffset，否则贴球折跃时舰首会捅进球里。
    //    抵达 SLIDE_ARRIVAL_K：材质要"完全从骨架之外出现"再滑进去，所以也要放大：
    //      K = 2 时材质的舰首恰好落在骨架的舰尾（总行程 = 一个舰长，两者正好不重叠）；
    //      K > 2 留出可见的间隙，读起来才是"从外面飞进来"，而不是"在原地变大"。
    //  两个倍率分开给，方便单独调。    
    inline constexpr float SLIDE_DEPART_K = 2.2f;    // 去程前冲
    inline constexpr float SLIDE_ARRIVAL_K = 2.5f;   // 抵达滑入

    inline float slideFactor(float tau)
    {
        const float s = slideAmount(tau);
        return (s < 0.0f) ? s * SLIDE_ARRIVAL_K : s * SLIDE_DEPART_K;
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
    inline constexpr float FRONT_GEO_START = TAU_STAR - SCAN_TIME;  // 几何剔除前沿起点（= 扫描起点）
    //inline constexpr float FRONT_RETREAT = 0.8f * T_APPEAR;     // 返程退回时长
    
    inline constexpr float FRONT_RETREAT = FILL_TIME;           // 返程退回时长（= 材质填充时长）

    //inline constexpr float FRONT_OVER = 1.02f;                // 越冲量，理由见 geoFront

    //  前沿的取值域要覆盖噪声的取值范围，而不是 [0,1]：
    //  gt = g + NOISE_W*(n - 0.5)，其中 n ∈ [0,1]、g ∈ [0,1]，所以 gt ∈ [-w/2, 1 + w/2]。
    //  取满这个范围之后"静止"与"完成"才是精确的端点语义：
    //    静止（r = 0）：几何前沿低于噪声下界，剔除条件恒不成立；
    //    完成（r = 1）：几何前沿高于噪声上界，剔除条件恒成立。
    inline constexpr float FRONT_LO = -0.5f * NOISE_W;          // -0.125
    inline constexpr float FRONT_HI = 1.0f + 0.5f * NOISE_W;    //  1.125
    inline constexpr float FRONT_LEAD = 0.25f;                  // 材质领先几何的空间量

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
    //  mix，残留偏移会一直带着走）。
    inline constexpr float FOV_KICK = 3.5f;                  // Φ，单位是度
    inline constexpr float SHAKE_AMT = 0.30f;                 // 世界单位
    inline constexpr float KICK_START = TAU_STAR - 0.07f;      // FOV 冲击起点
    inline constexpr float KICK_END = TAU_STAR + 0.30f;      // FOV 冲击归零点
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
    //  u_w 是梯形，末段会落下 —— 不能拿它当生长位置，否则线框会从艉向艏缩回去。
    //  所以把"位置"和"强度"分成两件事：
    //    位置 = 下面这个单调函数（只前进）
    //    强度 = wireAlpha（见下）
    //inline constexpr float WIRE_GROW_TIME = 0.20f;   // 自艏扫到艉所用的时间

    //inline float wireGrow(float tau)
    //{
    //    if (tau <= TAU_STAR) return 0.0f;
    //    if (tau >= TAU_STAR + WIRE_GROW_TIME) return 1.0f;
    //    return smoothstep01((tau - TAU_STAR) / WIRE_GROW_TIME);
    //}

    // ===== 阶段 5：线框的生长位置（计划书 §8 阶段 5）=====================
    //  生长位置 = 下面这根单调曲线（只前进）；强度 = wireAlpha（见下）。
    //  曲线自 tau* 起画，GROW_SPAN 秒画满落点的骨架，之后恒为完整 ——
    //  也就是说"自艉向艏"的揭示发生在**传送之后**，蓄能期不画骨架。
    //
    //  为什么必须挪到传送后：SC2 的落点严格排在"舰体完全消失"之后，蓄能期出现骨架
    //  就等于提前暴露落点。原版把生长放在蓄能期，是为了和"蓄能期蓝图"接力，
    //  蓝图删掉之后那条理由不再存在。
    //  材质重建（geoFront 的退回）在实体化段仍会自艏向艉揭示一次，不会丢。
    
    // inline constexpr float GROW_SPAN = 0.50f;                  // 骨架生长时长（上界见 SC 计划书命题 5）
    inline constexpr float WIRE_GROW_TIME = TAU_STAR + GROW_SPAN;

    inline float wireGrow(float tau)
    {
        if (tau <= TAU_STAR)       return 0.0f;
        if (tau >= WIRE_GROW_TIME) return 1.0f;
        return smoothstep01((tau - TAU_STAR) / GROW_SPAN);  // 平滑过渡版本
        //return tau / WIRE_GROW_TIME;  // 线性过渡版本

        //const float x = tau / WIRE_GROW_TIME;
        //return 1.0f - (1.0f - x) * (1.0f - x);  // 起步快收尾慢的过渡版本
    }


    // ===== SC 分支：抵达段的"整体凝实"因子 ==================================
    //  实体化段内由 0 升到 1，乘在平台密度上，把整段半透明一起拉回不透明。
    //  逐帧观察里抵达段是"透明舰体出现、随后透明度下降"：前半句由前沿反跑负责，
    //  后半句由这一条负责。
    //  为什么不跟着前沿走：跟随前沿的话，"半透明能保持多久"会被前沿速度钉死成
    //  L / 速度（抵达段只有 0.033 s，约一帧）；独立成包络之后可以自由定长。
    //  用 σ 而不是 φ：它是强度类量（决定抖散密度），按本工程的分配原则 C1 足够。
    inline constexpr float SOLIDIFY_START = T_WIRE_END;         // = 1.80，实体化段起点
    //inline constexpr float SOLIDIFY_TIME = T_APPEAR;            // = 实体化段全长
    inline constexpr float SOLIDIFY_TIME = FILL_TIME;           // 与材质填充同期完成

    inline float solidify(float tau)
    {
        if (tau <= SOLIDIFY_START) return 0.0f;
        if (tau >= SOLIDIFY_START + SOLIDIFY_TIME) return 1.0f;
        return smoothstep01((tau - SOLIDIFY_START) / SOLIDIFY_TIME);
    }
    
    // ===== 幽灵舰体强度：与骨架同步长出，长满之后很快退掉，只留骨架 =====
    //  不能用 1 - solidify(tau)：那条钉在材质填充期，材质出现之前恒为满亮，
    //  于是"骨架长满到材质出现"这一整段（现在有 2.97 秒）幽灵会一直亮着 ——
    //  而参考画面里这时候只剩线框。
    inline constexpr float GHOST_FADE = 0.50f;      // 长满之后退掉所用的时长
    inline float ghostAlpha(float tau)
    {
        return 1.0f - smoothstep01((tau - WIRE_GROW_TIME) / GHOST_FADE);
    }

    // ===== SC 分支：电流外壳自己的强度包络 ==================================
    //  为什么不直接挂 u_c：u_c 的下降沿写在互相咬合的通道表里（V_R0 = C_F0），
    //  动它会把白化与拉伸的起点一起拖走。外壳要能独立决定"什么时候开始暗、什么时候归零"，
    //  所以单给一条包络，**整条都用扫描进度表达**（与四段时长解耦）：
    //    升段：蓄能段内由 0 升到满，正好在扫描开始时到顶（τ = TAU_STAR - SCAN_TIME）；
    //    保持到 p = SHELL_FADE_P0，然后渐暗，到 p = SHELL_FADE_P1 归零。
    //  取 SHELL_FADE_P1 = 0.18：那正是"材质前沿刚越过艏"的进度（对照表见精读第 7 段），
    //  也就是【半透明波开始扫描的那一刻】。壳在那之前就退干净，绝不带进后面的相位。
    //inline constexpr float SHELL_FADE_P0 = 0.08f;              // 开始变暗的扫描进度
    //inline constexpr float SHELL_FADE_P1 = 0.18f;              // 归零的扫描进度（= 半透明波刚过艏）
        
    //  三段都用扫描进度 p 表达（p < 0 就是蓄能期）。
    //  SHELL_FADE_P1 不能往后拉：外壳 pass 没有溶解门，0.18 是"材质前沿刚到舰首"的硬边界，
    //  越过去就会画出一个"舰体已经没了、外壳还完整"的轮廓。
    //  要让渐暗够长，只能把"升满"和"开始渐暗"整体挪到蓄能期里。
    inline constexpr float SHELL_RISE_P0 = -1.00f;   // 开始出现
    inline constexpr float SHELL_RISE_P1 = -0.80f;   // 升满
    inline constexpr float SHELL_FADE_P0 = -0.55f;   // 开始渐暗（蓄能期内）
    inline constexpr float SHELL_FADE_P1 = 0.18f;   // 归零（= 半透明波刚过艏）


    //inline float shellAlpha(float tau)
    //{
    //    const float scanStart = TAU_STAR - SCAN_TIME;
    //    const float up = smoothstep01(tau / scanStart);        // 蓄能段升满，正好在扫描开始时到顶
    //    const float p = (tau - scanStart) / SCAN_TIME;         // 归一化进度（可以小于 0）
    //    if (p <= SHELL_FADE_P0) return up;
    //    if (p >= SHELL_FADE_P1) return 0.0f;
    //    return up * (1.0f - smoothstep01((p - SHELL_FADE_P0) / (SHELL_FADE_P1 - SHELL_FADE_P0)));
    //}

    inline float shellAlpha(float tau)
    {
        const float scanStart = TAU_STAR - SCAN_TIME;
        const float p = (tau - scanStart) / SCAN_TIME;         // 归一化进度（可以小于 0）
        if (p <= SHELL_RISE_P0) return 0.0f;
        if (p < SHELL_RISE_P1)
            return smoothstep01((p - SHELL_RISE_P0) / (SHELL_RISE_P1 - SHELL_RISE_P0));
        if (p <= SHELL_FADE_P0) return 1.0f;
        if (p >= SHELL_FADE_P1) return 0.0f;
        return 1.0f - smoothstep01((p - SHELL_FADE_P0) / (SHELL_FADE_P1 - SHELL_FADE_P0));
    }

    // ===== 线框（含蓄能期的蓝图）的强度 ==================================
    //  只保留 u_w 的【下降沿】：蓄能期就全亮、τ* 处不跳变、末段随材质取代而淡出。
    //  为什么不直接用 u_w：它的上升段是为"线框随传送出现"设计的；
    //  现在落点的骨架自蓄能期就在场，再淡入一次就会在 τ* 前后掉一次亮度 ——
    //  模式 1/2 没有全屏白遮着，那一掉就是一次闪断。
    //  下降沿仍引用 W_F0 / W_F1，所以淡出时机照样只在通道表里改一处。
    inline float wireAlpha(float tau)
    {
        if (tau <= 0.0f) return 0.0f;
        return 1.0f - smoothstep01((tau - W_F0) / (W_F1 - W_F0));
    }
}