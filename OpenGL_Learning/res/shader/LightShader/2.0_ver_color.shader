// VER 6.0  点光源
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoords;

// out vec3 FragPos;
// out vec3 Normal;
// out vec2 TexCoords;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
// 	FragPos = vec3(model * vec4(aPos, 1.0));
// 	Normal = mat3(transpose(inverse(model))) * aNormal;
// 	TexCoords = aTexCoords;

// 	gl_Position = projection * view * vec4(FragPos, 1.0);	
// }


// VER 7.0 - 7.1  聚光灯 + 衰减
// VER 8.0  混合光照
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
	FragPos = vec3(model * vec4(aPos, 1.0));	// 世界空间坐标
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TexCoords = aTexCoords;

	gl_Position = projection * view * vec4(FragPos, 1.0);	// gl_Position是裁剪空间坐标
}