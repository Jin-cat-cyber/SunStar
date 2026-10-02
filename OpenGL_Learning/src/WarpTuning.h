#pragma once
// ---------------------------------------------------------------------------
//  WarpTuning.h —— 折跃观感参数（只放"喂给 GPU 的数"）
//
//  三份参数的分工：
//    WarpSC.h        时间线、几何、幅度（纯数学，不含 glm / 不含 GL，能被自测覆盖）
//    WarpDebris.h    碎屑的运动学（同样是纯数学）
//    WarpTuning.h    着色器 uniform 的取值：颜色、强度、频率、宽度、厚度
//
//  最后一类单独放，是因为它的调试方式是"看画面"，而前两类是"看数字/看自测"；
//  混在一起时，调颜色要翻过一堆时间线常量，调时长又要翻过一堆颜色。
//  这里只放常量，需要乘运行时量的地方在 cpp 里现算（例如 shipBoundR * U_SHELL_WORLD_K）。
//
//  颜色用 inline const 而不是 constexpr：glm 的向量构造函数是否 constexpr 取决于
//  版本与宏开关，用 inline const 同样只有一份实例，且一定编得过。
// ---------------------------------------------------------------------------

#include <glm/glm.hpp>

namespace warp_tune
{
    // ===== 舰体 PBR：溶解、切口与边缘光 ====================================
    inline constexpr float U_WHITE_K = 6.0f;     // 白热系数
    inline constexpr float U_TRANSLUCENCY = 0.40f;    // 半透明平台的抖散密度（被丢弃的格子比例）
    inline constexpr float U_FRONT_SOFT = 0.03f;    // 半透明波前沿的过渡宽度（轴向单位，必须 > 0）
    inline constexpr float U_NOISE_FREQ = 9.0f;     // 溶解噪声频率（同时决定抖散格子大小）
    inline constexpr float U_EDGE = 0.06f;    // 切口发光带宽
    inline constexpr float RIM_BASE = 0.6f;     // 边缘光基数
    inline constexpr float RIM_CHARGE_GAIN = 4.0f;     // 蓄能期边缘光的额外倍数
    //inline constexpr float RIM_STRENGTH_RATE = 1.0f + RIM_CHARGE_GAIN;
    inline const glm::vec3 U_DISSOLVE_COLOR = glm::vec3(0.25f, 0.95f, 0.80f);   // 剥离区与切口颜色

    // ===== 光柱（复用碎屑的着色器，但常量分开：白带是要的效果，别为"避免糊"压低）==
    inline constexpr float U_PILLAR_GAIN = 0.90f;
    inline constexpr float U_PILLAR_EDGE = 0.10f;
    inline const glm::vec3 U_PILLAR_COLOR = glm::vec3(0.45f, 0.82f, 1.00f);

    // ===== 电流外壳（含多边形能量网格）=====================================
    inline constexpr float U_SHELL_GAIN = 0.01f;   // 整体强度（乘 shellAlpha）；0.01 经 gamma 约 22% 的灰，调到 0.5 会盖住舰体细节
    
    inline constexpr float U_SHELL_WORLD_K = 0.012f;   // 外壳厚度 = shipBoundR * K
    inline constexpr float U_GRID_FREQ = 12.0f;   // 网格密度（每单位归一化坐标的格数）
    inline constexpr float U_GRID_WIDTH = 0.035f;  // 格线宽度（单元格为单位，别超过 0.2）
    inline constexpr float U_GRID_MIX = 1.0f;    // 第二组 45° 格线权重：0 = 退回只有正方格
    inline constexpr float U_GRID_STRENGTH = 0.60f;   // 网格亮度
    inline const glm::vec3 U_SHELL_COLOR = glm::vec3(0.35f, 0.75f, 1.0f);

    // ===== 线框骨架 ========================================================
    inline constexpr float U_WIRE_WIDTH = 1.5f;    // 屏幕空间线宽（约等于像素）
    inline const glm::vec3 U_WIRE_COLOR = glm::vec3(0.35f, 0.90f, 1.00f);
    //  落点骨架专用的暗一档颜色：幽灵船退掉之后只剩它，那个亮度显眼。
    inline const glm::vec3 U_WIRE_COLOR_BONE = glm::vec3(0.22f, 0.55f, 0.62f);   // 约 0.62 倍

    // ===== 幽灵舰体 ========================================================
    inline constexpr float U_GHOST_GAIN = 1.2f;   // 乘 (1 - solidify)
    inline constexpr float U_GHOST_EXPAND_K = 0.01f;  // 外扩厚度 = shipBoundR * K
    inline constexpr float U_GHOST_GROW_SOFT = 0.10f;  // 生长前沿的软化宽度
    inline constexpr float U_GHOST_RIM_POW = 2.5f;   // 菲涅尔幂次
    inline constexpr float U_GHOST_BODY = 0.15f;  // 体色常数项（0 = 只有边缘亮）
    inline const glm::vec3 U_GHOST_COLOR = glm::vec3(0.30f, 0.80f, 1.00f);


    // ===== 碎屑 ============================================================
    inline constexpr float U_DEBRIS_STRENGTH = 1.0f;
    inline constexpr float U_DEBRIS_EDGE = 0.20f;  // 正对处的基础亮度
    inline const glm::vec3 U_DEBRIS_COLOR = glm::vec3(0.40f, 0.85f, 1.00f);

    // ===== 全屏闪现（白闪 / 拒绝红闪）======================================
    inline constexpr float U_FLASH_BASE = 6.0f;
    inline constexpr float U_FLASH_CORE = 10.0f;
    inline constexpr float U_FLASH_REJECT_BASE = 1.0f;
    inline constexpr float U_FLASH_REJECT_CORE = 0.8f;
    inline const glm::vec3 U_FLASH_COLOR = glm::vec3(1.0f);
    inline const glm::vec3 U_FLASH_REJECT_COLOR = glm::vec3(1.0f, 0.10f, 0.08f);
}
