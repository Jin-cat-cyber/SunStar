// VER 1.0
// #version 330 core
// layout (location = 0) in vec3 aPos;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
// 	gl_Position = projection * view * model * vec4(aPos, 1.0);
// }


// VER 2.0		世界空间中计算冯氏光照模型
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;

// out vec3 FragPos;
// out vec3 Normal;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;


// void main()
// {
// 	FragPos = vec3(model * vec4(aPos, 1.0));
// 	Normal = mat3(transpose(inverse(model))) * aNormal; // 法线矩阵
// 	//	inverse(model)：先对 model 取逆，抵消缩放和旋转的影响。
// 	//	transpose(...)：再转置，保证变换后的法线方向正确。
// 	//	mat3(...)：只取矩阵的前三行三列（因为法线是3D向量）。

// 	gl_Position = projection * view * model * vec4(aPos, 1.0);
// }


// VER 2.1		观察空间中计算冯氏光照模型(带有平行光)
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;

// out vec3 FragPos;
// out vec3 Normal;
// out vec3 LightPos;  // 额外的变量，因为我们需要视图空间中的灯光位置，所以我们在顶点着色器中进行计算。

// uniform vec3 lightPos;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
//     gl_Position = projection * view * model * vec4(aPos, 1.0);
//     FragPos = vec3(view * model * vec4(aPos, 1.0));
//     Normal = mat3(transpose(inverse(view * model))) * aNormal;
//     LightPos = vec3(view * vec4(lightPos, 1.0)); // 将光源位置由 世界空间坐标 变换到 观察空间坐标
// }


//VER 3.0  材质
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;

// out vec3 FragPos;
// out vec3 Normal;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
// 	FragPos = vec3(model * vec4(aPos, 1.0));
// 	Normal = mat3(transpose(inverse(model))) * aNormal;

// 	gl_Position = projection * view * vec4(FragPos, 1.0);
// }


// VER 4.0  漫反射贴图
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TexCoords = aTexCoords;

	gl_Position = projection * view * vec4(FragPos, 1.0);	
}