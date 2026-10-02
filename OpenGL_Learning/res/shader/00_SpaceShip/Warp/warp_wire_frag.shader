#version 330 core
out vec4 FragColor;

in vec3 gBary;     // 重心坐标：某个分量为 0 即落在对边上
in vec3 gObjPos;   // 物体空间坐标（生长门用）

uniform vec3  uWireColor;   // 阵营色挂点，单一颜色、不随阶段插值
uniform float uWireAlpha;   // 由 u_w 驱动：整体明暗（含末段淡出）
uniform float uWireGrow;    // 生长位置，单调 0 到 1（自艏向艉）
uniform float uWireWidth;   // 屏幕空间线宽，单位约等于像素
uniform float uAxisMinY;
uniform float uAxisMaxY;

void main()
{
    // 把重心坐标到三条边的"距离"用 fwidth 换算到屏幕空间，于是：
    //   · 线宽在屏幕上恒定，不随距离变化；
    //   · smoothstep 自带解析抗锯齿，1k 面数也不会闪成噪点。
    vec3  d    = fwidth(gBary) * uWireWidth;
    vec3  s    = smoothstep(vec3(0.0), d, gBary);
    float line = 1.0 - min(min(s.x, s.y), s.z);
    // 三个分量都远离 0 时 s 全为 1，line = 0 —— 三角形内部不画。

    // 生长门：轴向坐标 g 小于 uWireGrow 的部分才画（g = 0 在艏）。
    // 用 1 - smoothstep 是因为要的是"小于"那一侧。
    float g    = (gObjPos.y - uAxisMinY) / max(uAxisMaxY - uAxisMinY, 1e-6);
    float gate = 1.0 - smoothstep(uWireGrow - 0.06, uWireGrow, g);

    float a = line * gate * uWireAlpha;
    if (a <= 0.002) discard;    // 内部、还没生长到的、以及整体淡出之后都不画

    // 加法混合（GL_ONE, GL_ONE）：这里输出的是"要加多少"。
    FragColor = vec4(uWireColor * a, 1.0);
}