// #version 330 core
// layout (location = 0) out vec3 gPosition;
// layout (location = 1) out vec3 gNormal;
// layout (location = 2) out vec4 gAlbedoSpec;

// in vec2 TexCoords;
// in vec3 FragPos;
// in vec3 Normal;

// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;

// void main()
// {
// 	gPosition = FragPos;

// 	gNormal = normalize(Normal);

// 	gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;

// 	gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
// }


// VER 2.0 ·¨ÏßÌùÍ¼°æ±¾
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 WorldNormal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

void main()
{
	gPosition = FragPos;

	vec3 tangentNormal = texture(texture_normal1, TexCoords).rgb * 2.0 - 1.0;
	mat3 TBN = mat3(normalize(Tangent), normalize(Bitangent), normalize(WorldNormal));
	gNormal = normalize(TBN * tangentNormal);

	gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;

	gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}