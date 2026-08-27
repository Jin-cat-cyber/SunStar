#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBR;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 WorldNormal;

uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

void main()
{
    // gPosition = FragPos;
    // gNormal = normalize(WorldNormal);
    // gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // gAlbedoSpec.a   = texture(texture_specular1, TexCoords).r;

    vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;

    gPosition = FragPos;
    gNormal   = normalize(WorldNormal);
    gAlbedo   = vec4(albedo, 0.0);          // 行星岩石：非金属 metallic=0
    gPBR      = vec4(0.9, 1.0, 0.0, 0.0);   // roughness=0.9, ao=1.0
}