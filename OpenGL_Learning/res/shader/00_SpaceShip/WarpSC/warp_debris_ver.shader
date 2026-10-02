#version 330 core
// 碎屑的顶点着色器：顶点在 ±1，实例矩阵只含旋转与平移，非均匀缩放单独由 aHalf 给 ——
// 这样法线可以用 mat3(model * aInst) 精确变换，不必求逆转置。

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in mat4 aInst;   // 旋转 + 平移，不含缩放
layout (location = 6) in vec3 aHalf;   // 半尺寸（世界单位）：压扁某一轴就是矩形板
layout (location = 7) in vec4 aTint;   // rgb = 色偏，a = 存活强度

out vec3  vWorldPos;
out vec3  vNormal;
out vec3  vTint;
out float vAlpha;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;    // 碎屑锚点：折跃开始那一帧的舰体姿态，已还原到世界尺度

void main()
{
	vec4 world = model * aInst * vec4(aPos * aHalf, 1.0);

	vWorldPos = world.xyz;
	vNormal	  = normalize(mat3(model * aInst) * aNormal);
	vTint	  = aTint.rgb;
	vAlpha    = aTint.a;

	gl_Position = projection * view * world;
}