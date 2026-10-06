#version 330 core
// 线框专用的最小顶点着色器：只要物体空间坐标（生长门用）与裁剪空间位置。
layout (location = 0) in vec3 aPos;

out vec3 vObjPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vObjPos = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}