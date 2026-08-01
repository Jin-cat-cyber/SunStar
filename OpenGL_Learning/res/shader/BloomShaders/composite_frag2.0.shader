#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

// 参数（运行时可调）
uniform float exposure      = 1.0;
uniform float bloomStrength = 0.25;
uniform vec3  colorTint     = vec3(1.0);
uniform float gamma         = 2.2;
uniform float saturation    = 1.02;
uniform float contrast      = 1.06;
uniform vec3  whiteBalance  = vec3(0.99, 0.99, 1.01); // 微偏冷可去黄
uniform bool  useWarmHighlight = true;
uniform vec3  warmColor     = vec3(1.0, 0.82, 0.6);
uniform float warmStrength  = 0.12;
uniform float warmThreshold = 0.65;

// Uncharted2 filmic tonemapper
vec3 Uncharted2Tonemap(vec3 x)
{
    // Hable curve constants
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E / F;
}

vec3 AdjustSaturation(vec3 color, float sat)
{
    float gray = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(gray), color, sat);
}

vec3 AdjustContrast(vec3 color, float c)
{
    return (color - 0.5) * c + 0.5;
}

void main()
{
    // 读取 HDR 线性缓冲
    vec3 sceneColor = texture(sceneTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;

    // 合成 HDR（保留线性空间）
    vec3 hdr = sceneColor + bloomColor * bloomStrength * colorTint;

    // 曝光
    hdr *= exposure;

    // 应用 Uncharted2 filmic tone mapping（保持高光细节）
    vec3 curr = Uncharted2Tonemap(hdr);
    // 标准化到白点（使用常见白点值）
    const float W = 11.2; // white point
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
    vec3 mapped = curr * whiteScale;

    // 小幅暖色高光（避免过强）
    if (useWarmHighlight)
    {
        float lum = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
        float glow = smoothstep(warmThreshold, warmThreshold + 0.15, lum);
        mapped += warmColor * glow * warmStrength;
    }

    // 白平衡 / 饱和度 / 对比度
    mapped *= whiteBalance;
    mapped = AdjustSaturation(mapped, max(0.0, saturation));
    mapped = AdjustContrast(mapped, max(0.0, contrast));

    // 最后伽马到 sRGB
    mapped = pow(clamp(mapped, 0.0, 1.0), vec3(1.0 / max(gamma, 0.001)));

    FragColor = vec4(mapped, 1.0);
}