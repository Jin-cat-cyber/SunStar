// VER 3.0f
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
//uniform mat3 normalMatrix; // 从CPU计算并传入，替代在shader里inverse/transpose

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));	// 世界空间坐标
    //Normal = normalMatrix * aNormal; // 在片段着色器归一化
    Normal = mat3(transpose(inverse(model))) * aNormal; // 在片段着色器归一化
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f); 
}
