#version 330 core
// 电流外壳的顶点着色器：把同一套 mesh 沿法线外扩一小段再画一遍。
//
// 偏移在【世界空间】做。物体局部坐标比世界坐标大 2000 倍（GetModelMatrix 里
// 有 scale(0.0005)），在物体空间里定厚度极易搞错量级；而世界空间的厚度对
// 调用方来说就是一句"舰体半径的百分之几"。
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vObjPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float uShellWorld;   // 外壳厚度，世界单位

void main()
{
	vec4 world = model * vec4(aPos, 1.0);

	// 模型矩阵是"平移 * 旋转 * 均匀缩放"，所以 mat3(model) * aNormal 只差一个
    // 正标量，归一化之后方向就是对的 —— 不必再让调用方传 normalMatrix。
	vec3 n = normalize(mat3(model) * aNormal);

	world.xyz += n * uShellWorld;	
	
	vWorldPos = world.xyz;
	vNormal   = n;
	vObjPos   = aPos;        // 物体空间坐标：细丝图案绑在舰体上，不随舰体移动而游动

	gl_Position = projection * view * world;

}