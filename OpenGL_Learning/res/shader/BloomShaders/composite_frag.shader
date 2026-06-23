#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

uniform float exposure = 1.5;
uniform float bloomStrength = 0.5;
uniform vec3 hdrColor = vec3(1.0);

void main()
{
	vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
	vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
	hdrColor += bloomColor * bloomStrength;
	// 色调映射 （指数映射， 保留高光细节）
	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	// 最终颜色调整
	mapped *= hdrColor;
	FragColor = vec4(mapped, 1.0);
}