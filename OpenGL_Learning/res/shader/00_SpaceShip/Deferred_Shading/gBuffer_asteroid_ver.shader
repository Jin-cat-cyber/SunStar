#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;   // 实例化模型矩阵

out vec3 FragPos;
out vec2 TexCoords;
out vec3 WorldNormal;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    mat3 normalMatrix = transpose(inverse(mat3(instanceMatrix)));
    WorldNormal = normalMatrix * aNormal;
    gl_Position = projection * view * worldPos;
}