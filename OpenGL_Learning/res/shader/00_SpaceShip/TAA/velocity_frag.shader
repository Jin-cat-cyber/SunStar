#version 330 core
in vec4 vPrevClip;

uniform vec2 uScreenSize;

out vec4 FragColor;

void main()
{
    vec2 uvCur  = gl_FragCoord.xy / uScreenSize;
    vec2 uvPrev = (vPrevClip.xy / vPrevClip.w) * 0.5 + 0.5;
    //  a = 1 表示"这个像素有速度可用"。纹理清成 0，所以没画到的地方 a = 0 → TAA 走相机重投影。
    FragColor = vec4(uvPrev - uvCur, 0.0, 1.0);
}