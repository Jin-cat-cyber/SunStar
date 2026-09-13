#version 330 core

layout (location = 0) in vec4 aPosSize;   // xyz=世界位置, w=点径(像素)

uniform mat4 projection;
uniform mat4 view;
out vec3 WorldPos;

void main()
{
	WorldPos = aPosSize.xyz;
	gl_Position = projection * view * vec4(WorldPos, 1.0);
	gl_PointSize = aPosSize.w;			// CPU 已按屏幕占比算好
}