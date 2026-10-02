#pragma once
// ---------------------------------------------------------------------------
//  WarpPillar.h —— SC 分支折跃的光柱：长在舰体体积内的纵向发光丝，纯运动学。
//
//  一根光柱 = 一个沿舰体局部 Y 拉长的立方体。X / Z 是横截面（细），Y 是半长（长），
//  三个轴都是舰体局部轴，没有任何随机朝向。内端钉在出生点上，外端朝一侧伸长。
//
//  出生点取在【舰体体积内部】，这是有意的：柱子平时埋在舰体里，舰体溶解时被露出来，
//  材质恢复时被盖回去。露与盖完全由【深度测试】决定 —— 这一层加法混合、不写深度，
//  但保留深度测试、并且画在舰体 pass 之后，所以舰体还实的地方会把柱子挡掉，
//  被 discard 的地方没有深度、柱子就透出来。详见计划书第 2 节的命题 1。
//
//  两段窗口互为反过程：
//    去程：半透明波扫过时被露出来，缓缓向外漂移后消失。
//    抵达：骨架刚长满就出现（早于材质），材质出现一半时停止出生，材质填满时收干净。
//
//  坐标一律用世界单位，定在【当前这一帧的舰体逻辑位姿】里 —— 调用方传进来的矩阵是
//  shipWireModel（不含滑入与拉伸），所以抵达段柱子钉在落点不动、材质从后方滑过来接上。
//  注意：该矩阵在 tau* 那一帧会跳变（舰体被传送），所以去程窗口必须在 tau* 之前收干净；
//  P_DEPART_T1 = scanTau(1.0f) = TAU_STAR 与 pillarAt 里的 min 夹取就是为此。
// ---------------------------------------------------------------------------
#include "WarpDebris.h"     // 复用 dRand / dLerp / dClamp01 / dSmooth

namespace warp_sc
{
    // ===== 光柱参数 ========================================================
        
    //  去程已接线（2026-09-28）：碎屑在 24.A 退场之后由它接管。
    inline constexpr bool  P_DEPART_ENABLE = true;
    inline constexpr float P_DEPART_T1 = scanTau(1.0f);                         // = TAU_STAR，见文件头。

    //inline constexpr float P_ARRIVE_T0 = WIRE_GROW_TIME;                      // 1.60，骨架刚长满。
    inline constexpr float P_ARRIVE_T0 = WIRE_GROW_TIME + HOLD_TIME;            // 光柱出现 = 骨架长满 + 静止等待。
    
    inline constexpr float P_ARRIVE_BIRTH_T1 = T_WIRE_END + 0.5f * FILL_TIME;   // 1.95，材质出现一半。
    //inline constexpr float P_ARRIVE_T1 = T_WIRE_END + FILL_TIME;              // 2.10，全部收干净。
    inline constexpr float P_ARRIVE_T1 = T_WIRE_END + FILL_TIME + 0.23f;        // 材质填满后再残留 0.23 秒。

    inline constexpr int   P_COUNT = 64;                // 抵达段的根数。
    inline constexpr int   P_DEPART_COUNT = 36;         // 去程段的根数（比抵达少：它要铺满整条舰体）。
    //inline constexpr float P_LIFE = 0.30f;            // 寿命上限（秒），再夹到窗口末。
    inline constexpr float P_LIFE = 0.50f;              // 寿命上限（秒），再夹到窗口末。

    //  前沿跨度：geoFront 的取值域宽度（含噪声两侧与材质领先量）。推导见计划书第 4 节。
    inline constexpr float P_FRONT_SPAN = FRONT_HI - FRONT_LO + FRONT_LEAD;   // = 1.5

    inline constexpr float P_R_MIN = 0.45f;       // 横截面半宽下界（世界单位）。
    inline constexpr float P_R_MAX = 1.10f;       // 上界（全宽 0.9 ~ 2.2，读作"柱"而不是"丝"）。
    inline constexpr float P_LEN_MIN = 1.2f;      // 轴向半长下界（生长完成后）。
    //inline constexpr float P_LEN_MAX = 3.8f;      // 上界：舰长的 1/8，即最长的一根全长为舰长的 1/4。
    inline constexpr float P_LEN_MAX = 6.7f;      // 上界：光柱全长为舰长的 1/4（朝折跃方向那批最终达到）。
    //  抵达段：柱长还随【出生先后】按方向演化 —— 朝折跃方向（dirY = -1）的越来越长，
    //  背向（dirY = +1）的越来越短。w 是该柱在出生窗里的归一化出生时刻（0 最早、1 最晚）。
    inline constexpr float P_FWD_LEN_K0 = 0.40f;    // 最早那批朝前柱的倍率
    inline constexpr float P_BACK_LEN_K1 = 0.50f;   // 最晚那批背向柱的倍率（0.5 正好给到舰长 1/8）
    
    inline constexpr float P_SPAWN_W = 0.30f;     // 出生点横向半程（× 舰体半径）：横向铺开覆盖舰体截面。
    //  轴向半程必须按段分开：抵达的柱子要挤在"逻辑面"那张薄板上；
    //  去程的必须沿整个舰体铺开，否则"随半透明波扫过依次出现"无从谈起。
    inline constexpr float P_ARRIVE_SPAWN_L = 0.12f;
    inline constexpr float P_DEPART_SPAWN_L = 0.95f;

    //  去程的尺寸倍率：参考画面里去程的分解光柱明显比抵达那批小，所以这里往小取。
    inline constexpr float P_DEPART_R_SCALE = 0.55f;
    inline constexpr float P_DEPART_LEN_SCALE = 0.55f;

    //  横向采样的幂律指数：越大越向轴心集中（见下一节）。
    inline constexpr float P_CORE_K = 2.2f;

    //  位移按【总位移】给，不按速度：柱子寿命长短不一，按速度给会让寿命长的漂得远。
    inline constexpr float P_DISP_AXIAL_MIN = 1.0f;
    inline constexpr float P_DISP_AXIAL_MAX = 3.0f;
    inline constexpr float P_DISP_OUT_MIN = 0.8f;
    inline constexpr float P_DISP_OUT_MAX = 2.4f;

    //  方向分界（世界单位，乘 bowR 得到阈值；0 = 中垂面）。
    //    伸长恒按中垂面分：前面朝舰首伸、后面朝舰尾伸。
    //    去程的轴向漂移单独一个分界：0 = 中垂面（前后各半），0.50 = 舰尾 1/4 向后。
    //inline constexpr float P_DIR_Y_K = 0.0f;
    //inline constexpr float P_DIR_Y_K = -1.0f;   // 【极端测试】全部朝舰尾伸（舰首那侧应该完全是空的）
    inline constexpr float P_DIR_Y_K = 0.0f;    // 伸长分界：中垂面。之前（舰首侧）朝前伸、之后（舰尾侧）朝后伸。
    
    inline constexpr float P_DRIFT_Y_K = 0.50f;
    //  抵达的轴向漂移：统一向舰首（局部 -Y，即折跃前进方向），不分向。
    inline constexpr float P_ARRIVE_AXIAL_DIR = -1.0f;
    //  光柱那张"面"相对落点的方位：-1 = 落点后方（材质出现的位置，当前），+1 = 落点前方。
    inline constexpr float P_PLANE_SIDE = -1.0f;

    //  长度生长与淡入淡出都按【自身寿命的比例】给：两段窗口长短差很多，绝对秒数会
    //  吃掉短窗口的寿命。约束：P_GROW_F <= 1，且 P_FADE_IN_F + P_FADE_OUT_F <= 1。
    inline constexpr float P_GROW_F = 0.35f;
    inline constexpr float P_FADE_IN_F = 0.20f;
    inline constexpr float P_FADE_OUT_F = 0.20f;
    //  末尾"缩掉"占寿命的比例。约束：P_GROW_F + P_SHRINK_F <= 1，否则还没长满就开始缩。
    inline constexpr float P_SHRINK_F = 0.40f;

    //  最后一个窗口结束的时刻：绘制门控用它。
    inline constexpr float P_END = P_ARRIVE_T1;

    // ===== 前沿反解 ========================================================
    //  半透明波（matFront）扫到轴向坐标 g 的时刻。推导见计划书第 4 节：
    //  matFront(tau) = FRONT_LO + P_FRONT_SPAN * r(tau)，令其等于 g 解出 r，
    //  再用 smoothstep 反函数的标准恒等式得到归一化时间 x。
    //  g = 0（舰首）给 0.6065 秒、g = 1（舰尾）给 0.9042 秒，不需要数值迭代。
    inline float pillarFrontTau(float g)
    {
        const float r = dClamp01((g - FRONT_LO) / P_FRONT_SPAN);
        const float x = 0.5f - std::sin(std::asin(1.0f - 2.0f * r) / 3.0f);
        return FRONT_GEO_START + x * SCAN_TIME;
    }

    //  世界单位轴向坐标换成归一化坐标 g（0 = 舰首、1 = 舰尾）。
    //  探针实测原点正好在舰体正中，所以直接用半舰长换算即可。
    inline float pillarGFromY(float y, float bowR)
    {
        return dClamp01(0.5f + 0.5f * y / (bowR > 1e-4f ? bowR : 1e-4f));
    }

    // ===== 一根光柱 ========================================================
    struct Pillar
    {
        float t0;        // 出生时刻（秒）。
        float life;      // 寿命：从出生到所在窗口结束。
        float p0[3];     // 出生点（世界单位，舰体体积内）。
        float half[3];   // 半尺寸：x / z 是横截面，y 是轴向半长（生长完成的长度）。
        float disp[3];   // 整个寿命里的总位移（世界单位）。
        float dirY;      // 轴向伸长方向：+1 向舰尾、-1 向舰首。
        float tint;      // 色偏，0 = 偏青、1 = 偏白。
    };

    //  arrival = false 取去程窗口，true 取抵达窗口。
    inline Pillar pillarAt(int i, float bowR, bool arrival)
    {
        const float w1 = arrival ? P_ARRIVE_T1 : P_DEPART_T1;
        const int   kb = arrival ? 50 : 30;   // 哈希通道基号：碎屑占了 1..18，两段窗口再各占一段。

        Pillar p;
        //  出生点：舰体包围盒内的一点。柱子埋在体内，所以横向要与碎屑同量级地铺开。
        //  轴向均匀取：去程要沿整个舰体铺开，抵达挤在"逻辑面"那张薄板上。
        const float spawnL = arrival ? P_ARRIVE_SPAWN_L : P_DEPART_SPAWN_L;
        p.p0[1] = (dRand(i, kb + 2) * 2.0f - 1.0f) * spawnL * bowR;

        //  横向取"半径 + 角度"，半径按幂律 u^P_CORE_K 分布：K > 1 时向轴心集中，
        //  柱子在中轴附近叠得更密，加法混合自然叠出亮核 —— 中心亮度就是这么来的。
        {
            const float u = dRand(i, kb + 3);
            const float ang = dRand(i, kb + 11) * 6.2831853f;
            const float rad = std::pow(u, P_CORE_K) * P_SPAWN_W * bowR;
            p.p0[0] = std::cos(ang) * rad;
            p.p0[2] = std::sin(ang) * rad;
        }

        //  出生时刻：抵达段在出生窗里铺开；去程段由半透明波扫到的时刻决定（前沿反解）。
        if (arrival)
        {
            const float u = dRand(i, kb + 0);
            p.t0 = P_ARRIVE_T0 + (P_ARRIVE_BIRTH_T1 - P_ARRIVE_T0) * u;
        }
        else
        {
            p.t0 = pillarFrontTau(pillarGFromY(p.p0[1], bowR));
        }
        //  寿命一律夹到窗口末：去程必须赶在 tau* 之前收干净（锚点矩阵会跳变）。
        const float rest = w1 - p.t0;
        p.life = (rest < P_LIFE) ? rest : P_LIFE;

        //  轴向伸长方向：远离中垂面（+Y 是舰尾、-Y 是舰首）。
        //  只有朝最近那一端伸，柱子才会探出舰体两端，读作"从舰体里长出来"。
        p.dirY = (p.p0[1] >= P_DIR_Y_K * bowR) ? 1.0f : -1.0f;

        //  横截面细、轴向长 —— 长宽比就是"丝"的读法来源。
        //  横截面与轴向长度。去程乘一组倍率：参考里去程的分解光柱比抵达那批明显小。
        //  所以只有放大才能与抵达读出同样的大小。
        const float rScale = arrival ? 1.0f : P_DEPART_R_SCALE;
        const float lScale = arrival ? 1.0f : P_DEPART_LEN_SCALE;
        const float r = dLerp(P_R_MIN, P_R_MAX, dRand(i, kb + 4));
        p.half[0] = r * rScale;
        p.half[2] = r * dLerp(0.7f, 1.3f, dRand(i, kb + 5)) * rScale;   // 截面不必是正方形。
        p.half[1] = dLerp(P_LEN_MIN, P_LEN_MAX, dRand(i, kb + 6)) * lScale;

        //  轴向位移：抵达统一向舰首；去程按漂移分界分向（与伸长是两条独立的分界）。
        const float axialDir = arrival
            ? P_ARRIVE_AXIAL_DIR
            : ((p.p0[1] >= P_DRIFT_Y_K * bowR) ? 1.0f : -1.0f);
        p.disp[1] = axialDir * dLerp(P_DISP_AXIAL_MIN, P_DISP_AXIAL_MAX, dRand(i, kb + 8));

        //  径向位移：从中轴向外。出生点几乎落在轴线上时用哈希方向顶上（与碎屑同款退化处理）。
        float rx = p.p0[0], rz = p.p0[2];
        float rl = std::sqrt(rx * rx + rz * rz);
        if (rl < 1e-4f)
        {
            const float ang = dRand(i, kb + 10) * 6.2831853f;
            rx = std::cos(ang); rz = std::sin(ang); rl = 1.0f;
        }
        const float outDisp = dLerp(P_DISP_OUT_MIN, P_DISP_OUT_MAX, dRand(i, kb + 9));
        p.disp[0] = rx / rl * outDisp;
        p.disp[2] = rz / rl * outDisp;

        p.tint = dRand(i, kb + 7);
        return p;
    }

    // ===== 随时间变化的三件东西 ============================================
    inline float pillarPhase(const Pillar& p, float tau)
    {
        return dClamp01((tau - p.t0) / (p.life > 1e-4f ? p.life : 1e-4f));
    }

    //  长度：从零长到全长，"从舰体里长出来"。
    inline float pillarLengthScale(float phase)
    {
        return dSmooth(dClamp01(phase / P_GROW_F));
    }

    //  抵达段专用的"方向 + 先后"长度倍率：朝折跃方向那批越来越长（到全长 = 舰长 1/4），
    //  背向那批越来越短（到一半 = 舰长 1/8）。
    //  用【出生先后】而不是寿命相位，所以每根柱子的长度在它自己的一生里是固定的 ——
    //  "随时间"体现在后出生的更长/更短，而不是每根各自长大缩小。
    inline float pillarDirLenScale(float w, float dirY)
    {
        const float e = dSmooth(dClamp01(w));
        return (dirY < 0.0f) ? dLerp(P_FWD_LEN_K0, 1.0f, e)
            : dLerp(1.0f, P_BACK_LEN_K1, e);
    }

    //  消失用"缩掉"而不是"淡掉"：寿命只有 0.15 到 0.30 秒，只靠淡化会读成闪一下。
    //  1 到 0，作用在整根柱子的三个轴上 —— 外端缩回内端，同时整根变细。
    inline float pillarShrinkScale(float phase)
    {
        const float x = dClamp01((phase - (1.0f - P_SHRINK_F)) / P_SHRINK_F);
        return 1.0f - dSmooth(x);
    }

    //  位移的时间形状：两端一阶导为零的缓入缓出，对应"缓缓移动"。
    //  不能用碎屑那条 debrisBurst（它是"被甩出去"，起步最快）。
    inline float pillarDrift(float phase) { return dSmooth(phase); }

    //  亮度：淡入乘淡出，两段比例都相对自身寿命。
    inline float pillarAlpha(const Pillar& p, float tau)
    {
        const float t = tau - p.t0;
        if (t <= 0.0f) return 0.0f;
        const float up = dSmooth(t / (P_FADE_IN_F * p.life));
        const float down = dSmooth((t - (1.0f - P_FADE_OUT_F) * p.life) / (P_FADE_OUT_F * p.life));
        return up * (1.0f - down);
    }

    //  该窗口此刻是否可能有活着的柱子（寿命都在窗口末结束，所以窗口即活动区间）。
    inline bool pillarWindowLive(float tau, bool arrival)
    {
        if (arrival) return tau >= P_ARRIVE_T0 && tau < P_ARRIVE_T1;
        //  去程：最早一批在舰首被扫到时出生，最晚一批在 tau* 收干净。
        return tau >= pillarFrontTau(0.0f) && tau < P_DEPART_T1;
    }
}

// ===== GL 侧：实现在 WarpDebris.cpp =====================================
//  与碎屑共用同一份立方体 mesh 与实例缓冲，所以不新建 .cpp（避开 vcxproj 登记那一关）。
class Shader;
void WarpPillarDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR, const glm::mat4& logicalModel);
