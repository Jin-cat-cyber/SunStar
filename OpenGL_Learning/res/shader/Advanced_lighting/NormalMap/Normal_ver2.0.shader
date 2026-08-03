#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
	vec3 TangentDirLightDir;	// 切空间定向光方向
	vec3 TangentSpotLightDir;	// 切空间聚光灯方向
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 dirLightDir;
uniform vec3 spotLightDir;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.TexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * aTangent);
	vec3 N = normalize(normalMatrix * aNormal);
	T = normalize(T - dot(T, N) * N);	// 偏移量矫正，使切线与法线正交
	vec3 B = cross(N,T);
	// vec3 B = normalize(normalMatrix * aBitangent);
	// B = normalize(B - dot(B, N) * N - dot(B, T) * T);	// 偏移量矫正，使副切线与法线、切线正交
	// // 方向再次矫正，保证切线、副切线、法线三者之间的关系正确
	// if(dot(B, cross(N, T)) < 0.0)
	// {
	// 	B = -B;
	// }

	// 构建切线空间矩阵
	mat3 TBN = transpose(mat3(T, B, N));
	vs_out.TangentLightPos	= TBN * lightPos;
	vs_out.TangentViewPos	= TBN * viewPos;
	vs_out.TangentFragPos	= TBN * vs_out.FragPos;
	vs_out.TangentDirLightDir	= TBN * normalize(-dirLightDir);
	vs_out.TangentSpotLightDir = TBN * normalize(-spotLightDir);

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}