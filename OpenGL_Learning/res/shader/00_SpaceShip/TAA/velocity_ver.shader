#version 330 core
//  TAA 速度 prepass 的顶点着色器：只画舰体，输出"上一帧的裁剪空间位置"。
//  不在这里除 w：uv 是投影函数，插值必须带 w 才是透视正确的，所以放到片元里除。
layout (location = 0) in vec3 aPos;

uniform mat4 uVPCur;      // 本帧 VP（含抖动）
uniform mat4 uModelCur;   // 本帧舰体模型矩阵（spaceshipModel）
uniform mat4 uModelPrev;  // 上一帧舰体模型矩阵
uniform mat4 uVPPrev;     // 上一帧 VP（含抖动）

out vec4 vPrevClip;

void main()
{
    vPrevClip = uVPPrev * uModelPrev * vec4(aPos, 1.0);
    gl_Position = uVPCur * uModelCur * vec4(aPos, 1.0);
}