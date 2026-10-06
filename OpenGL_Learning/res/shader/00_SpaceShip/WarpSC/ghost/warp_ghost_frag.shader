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

    FragColor = vec4(col * gate * uGhostStrength, 1.0);
}