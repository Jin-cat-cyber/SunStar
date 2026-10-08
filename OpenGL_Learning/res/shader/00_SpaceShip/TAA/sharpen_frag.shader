#version 330 core
//  TAA 写回 hdrColorBuffer 时的反锐化：TAA 的时间累积天然偏软（历史是带小数偏移的双线性采样，
//  逐帧重采样等于连续两次滤波），这里用 4 邻域拉普拉斯把局部对比找回来。
in vec2 TexCoords;

uniform sampler2D uImage;       // TAA resolve 的结果（历史 write 那张）
uniform vec2  uScreenSize;
uniform float uSharpen;         // 强度（0 = 关）

out vec4 FragColor;

void main()
{
    vec2 texel = 1.0 / uScreenSize;
    vec3 c = texture(uImage, TexCoords).rgb;
    vec3 n = texture(uImage, TexCoords + vec2( 0.0,  1.0) * texel).rgb;
    vec3 s = texture(uImage, TexCoords + vec2( 0.0, -1.0) * texel).rgb;
    vec3 w = texture(uImage, TexCoords + vec2(-1.0,  0.0) * texel).rgb;
    vec3 e = texture(uImage, TexCoords + vec2( 1.0,  0.0) * texel).rgb;

    vec3 blur  = (n + s + w + e) * 0.25;
    vec3 sharp = c + uSharpen * (c - blur);

    //  夹到非负：HDR 里的负值会在后续 bloom/tonemap 里变成 NaN 或黑边。
    FragColor = vec4(max(sharp, vec3(0.0)), 1.0);
}