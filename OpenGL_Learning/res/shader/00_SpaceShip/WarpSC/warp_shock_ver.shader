#version 330 core
// 冲击波（平面圆环）的顶点着色器：非实例化。
// 网格是【单位圆盘】：半径 1、躺在 XZ 平面里、法线恒为 +Y。
// 于是锚点的 Y 轴（舰体纵轴）正好是这张圆盘的法线 ——
// "环躺在一张垂直于纵轴的平面里"这件事由【构造】保证，不靠任何参数。

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3  vWorldPos;
out vec3  vNormal;
out vec3  vObjPos;     // 物体空间坐标：片元用它算世界半径，从而把环带以外的部分丢掉
out vec3  vTint;
out float vAlpha;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;        // 锚点 × scale(环半径, 1, 环半径)
uniform vec3 uShockTint;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);

    vWorldPos = world.xyz;

    //  缩放是 (r, 1, r) 的非均匀缩放，而圆盘的法线是 +Y —— 只被 Y 的缩放影响（此处为 1），
    //  方向永远不变，normalize 只是保底。
    vNormal = normalize(mat3(model) * aNormal);

    vObjPos = aPos;
    vTint   = uShockTint;
    vAlpha  = 1.0;          // 强度全部走 uDebrisStrength

    gl_Position = projection * view * world;
}