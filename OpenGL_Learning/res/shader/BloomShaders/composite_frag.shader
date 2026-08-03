// VER 1.0

// #version 330 core
// in vec2 TexCoords;

// out vec4 FragColor;

// uniform sampler2D sceneTexture;
// uniform sampler2D bloomTexture;

// uniform float exposure = 1.5;
// uniform float bloomStrength = 0.5;
// uniform vec3 colorTint = vec3(1.0);

// void main()
// {
// 	vec3 sceneColor = texture(sceneTexture, TexCoords).rgb;
// 	vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
// 	vec3 hdr = sceneColor + bloomColor * bloomStrength * colorTint;
// 	// 色调映射 （指数映射， 保留高光细节）
// 	vec3 mapped = vec3(1.0) - exp(-hdr * exposure);
// 	// 最终颜色调整
// 	FragColor = vec4(mapped, 1.0);
// }


// VER 2.0
#version 330 core
in vec2 TexCoords；

out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float exposure = 1.0;
uniform float bloomStrength = 0.5;
uniform vec3 colorTint = vec3(1.0);

vec3 ACES_Tonemap(vec3 x)
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;

	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() 
{
	vec3 sceneColor = texture(sceneTexture, TexCoords).rgb;
	vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
	// HDR 合成
	vec3 hdr = sceneColor + bloomColor * bloomStrength * colorTint;
	// 曝光控制
	hdr *= exposure;
	// ACRS映射
	vec3 mapped= ACES_Tonemap(hdr);

	float luminance = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
    vec3 warmHighlight = vec3(1.0, 0.7, 0.3);          // 炽热金色
    float glow = smoothstep(0.5, 0.9, luminance);      // 只有较亮区域受影响
    mapped = mix(mapped, mapped * warmHighlight, glow * 0.4);  // 40% 暖色混合
	FragColor = vec4(mapped, 1.0);
}