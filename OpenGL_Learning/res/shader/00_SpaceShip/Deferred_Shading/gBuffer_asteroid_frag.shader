#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 WorldNormal;

uniform sampler2D material_diffuse;   // 小行星贴图

void main()
{
    gPosition = FragPos;
    gNormal = normalize(WorldNormal);
    vec4 texColor = texture(material_diffuse, TexCoords);
    gAlbedoSpec.rgb = texColor.rgb;
    gAlbedoSpec.a   = 0.3;  // 小行星固定镜面强度
}