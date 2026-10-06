#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vObjPos;

uniform vec3  camPos;
uniform vec3  uGhostColor;     // 落点的舰体色，与线框同色系
uniform float uGhostStrength;  // 由"生长门 × 材质还没接上的程度"驱动
uniform float uAxisMinY;       // 舰体轴向范围（物体空间）
uniform float uAxisMaxY;
uniform float uGrow;           // 生长位置，单调 0 到 1，与线框同一个值
uniform float uGrowSoft;       // 生长前沿的软化宽度（轴向坐标单位）
uniform float uRimPow;         // 边缘白的菲涅尔幂次
uniform float uBody;           // 体色常数项：0 = 只有边缘，越大越"厚"

// ===== 折跃：抵达段的多道扫描波（7 道，周期 = 1 舰长）=====
uniform float uBandPhase;      // 波列相位：每 +1 扫过一整舰长，由 tau 与 SCAN_TIME 换算
//uniform float uBandW;          // 旧：归一化半宽（舰长 = 1），世界空间，屏幕宽度随视角与距离变。
uniform float uBandPx;         // 单道波的屏幕空间半宽（像素），全宽 = 2 倍本值。
uniform vec3  uBandStrong;     // 强档幅度：幽灵舰体最亮那档
uniform vec3  uBandWeak;       // 弱档幅度：线框那档
uniform float uBandGate;       // 总闸：与线框同生共死（wireAlpha）


void main()
{
    // 生长门与线框完全同构：轴向坐标镜像一次（g = 0 在艏，镜像后 0 在艉），
    // 于是落点自艉向艏长出。用 1 - smoothstep 是因为要的是"小于前沿"那一侧。
    float g    = (vObjPos.y - uAxisMinY) / max(uAxisMaxY - uAxisMinY, 1e-6);
    float gRev = 1.0 - g;
    float gate = 1.0 - smoothstep(uGrow - uGrowSoft, uGrow, gRev);
    if (gate <= 0.002) discard;      // 还没长到的地方整块不画，省掉填色

    vec3 V = normalize(camPos - vWorldPos);

    // 菲涅尔：掠射角亮、正对处暗。取绝对值是因为本工程从不启用面剔除，
    // 背面也会被画到，而背面正是"能看见内部结构"的来源。
    float fres = pow(1.0 - abs(dot(normalize(vNormal), V)), uRimPow);

    // 加法混合（GL_ONE, GL_ONE），所以这里输出的是"要加多少"，不是最终颜色。
    vec3 col = uGhostColor * (uBody + fres * 1.3);

    //FragColor = vec4(col * gate * uGhostStrength, 1.0);

    // ===== 多道扫描波 =====
    //  周期恰好 1 舰长，于是直接挂在上面那条归一化轴向坐标 g 上：相位每 +1，
    //  整列正好平移一整舰长，所以 g 增大的方向就是"从头到尾"。
    //  与骨架、幽灵的 gRev = 1 - g 相反（那两个是自艉向艏），别照抄。
    //  位置落在 1/20 舰长网格上：{0, 2, 4, 8, 12, 16, 17} / 20，其中第 3、7 道是弱档。
    float bandPos[7]    = float[7](0.00, 0.10, 0.20, 0.40, 0.60, 0.80, 0.85);
    float bandStrong[7] = float[7](1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0);

    float bu   = fract(g - uBandPhase);
    vec3  band = vec3(0.0);
    for (int i = 0; i < 7; ++i)
    {
        // 折回 [-0.5, 0.5)：这样压在周期接缝上的那一道会在艏、艉各出半个，
        // 而不是在接缝处凭空消失一道。
        float du = bu - bandPos[i];
        du -= floor(du + 0.5);
        //float xb = du / max(uBandW, 1e-6);
        //band += mix(uBandWeak, uBandStrong, bandStrong[i]) * exp(-xb * xb);

        // 宽度用 fwidth(g) 量到屏幕空间，与线框量 fwidth(gBary) 是同一套度量：
        // 无论相机多远、视角多斜，这道波在屏幕上恒为 2 * uBandPx 像素宽。
        // 【旧】世界空间写法（一份数值在不同视角下差几十倍，近看 20 多像素、顺轴看只剩 1 像素）：
        //float xb = du / max(uBandW, 1e-6);
        //band += mix(uBandWeak, uBandStrong, bandStrong[i]) * exp(-xb * xb);
        float hw = max(fwidth(g) * uBandPx, 1e-8);
        band += mix(uBandWeak, uBandStrong, bandStrong[i]) * (1.0 - smoothstep(0.0, 1.0, abs(du) / hw));
    }

    // 只在朝向相机的那一侧的壳上留效果：本工程从不启用面剔除，背面也会被着色，
    // 所以这里用 max 而不是 abs —— abs 会把背面一起点亮，读作贯穿舰体的发光切片。
    float face = max(dot(normalize(vNormal), V), 0.0);

    //FragColor = vec4(col * gate * uGhostStrength, 1.0);
    FragColor = vec4(col * gate * uGhostStrength + band * gate * face * uBandGate, 1.0);
}