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
uniform float time;

void main()
{
    // 计算世界空间位置与法线
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    // 简化的法线计算（日冕不需要精确法线）
    Normal = normalize(mat3(model) * aNormal); // 简化，避免昂贵的逆矩阵

    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}