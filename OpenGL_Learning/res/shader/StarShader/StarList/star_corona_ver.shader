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
    // // 日冕有轻微的波动效果
    // float wave = sin(time * 3.0 + aPos.y * 10.0) * 0.02;
    // vec3 pos = aPos * (1.0 + wave);
    
    // // 计算世界空间位置与法线（法线归一化）
    // vec4 worldPos = model * vec4(pos, 1.0);
    // FragPos = worldPos.xyz;

    // mat3 normalMatrix = transpose(inverse(mat3(model)));
    // Normal = normalize(normalMatrix * aNormal);

    // TexCoords = aTexCoords;

    // gl_Position = projection * view * worldPos;

    // 日冕有轻微的波动效果 - 修复：只影响表面，不影响整体大小
    float wave = sin(time * 3.0 + aPos.y * 5.0) * 0.005; // 减小幅度和频率
    
    // 修复：法线方向的轻微偏移，而不是整体缩放
    vec3 offset = normalize(aPos) * wave; // 沿着法线方向偏移
    vec3 pos = aPos + offset;            // 添加到原始位置
    
    // 计算世界空间位置与法线
    vec4 worldPos = model * vec4(pos, 1.0);
    FragPos = worldPos.xyz;

    // 简化的法线计算（日冕不需要精确法线）
    Normal = normalize(mat3(model) * aNormal); // 简化，避免昂贵的逆矩阵

    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}