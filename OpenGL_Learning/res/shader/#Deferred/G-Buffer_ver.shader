// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoords;

// out vec3 FragPos;
// out vec2 TexCoords;
// out vec3 Normal;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
// 	vec4 worldPos = model * vec4(aPos, 1.0);
// 	FragPos = worldPos.xyz;
// 	TexCoords = aTexCoords;

// 	mat3 normalMatrix = transpose(inverse(mat3(model)));
// 	Normal = normalMatrix * aNormal;

// 	gl_Position = projection * view * worldPos;
// }

// VER 2.0 法线贴图版本
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;       // 新增
layout (location = 4) in vec3 aBitangent;     // 新增

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Tangent;        // 新增，传给片元
out vec3 Bitangent;      // 新增
out vec3 WorldNormal;    // 新增，替代原来的 Normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	FragPos = worldPos.xyz;
	TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);         // Gram-Schmidt orthogonalization
    vec3 B = cross(N, T);

    Tangent    = T;
    Bitangent  = B;
    WorldNormal = N;

    gl_Position = projection * view * worldPos;
}