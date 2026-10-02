#version 330 core
// 全屏 quad 的顶点着色器。与 atmo_ver.shader 同一套约定：
// quadVAO 的属性 0 就是 NDC 坐标 vec2。
layout (location = 0) in vec2 aPos;

out vec2 vNDC;          // aPos 本身就是 NDC 坐标（-1..1），直接传给片元当"屏幕位置"

void main()
{
    vNDC = aPos;
    gl_Position = vec4(aPos, 0.0, 1.0);
}