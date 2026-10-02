#version 330 core
out vec4 FragColor;

in vec2 vNDC;

uniform float uFlash;        // 闪现强度（已按模式门控，模式 1/2 传 0）
uniform float uFlashBase;    // 整屏基底亮度：保证画面边缘也被压白
uniform float uFlashCore;    // 中心附加亮度：让 bloom 有一个圆心
uniform vec2  uFlashCenter;  // 舰体的屏幕位置（NDC）
uniform vec3  uFlashColor;   // 折跃白闪传白，拒绝反馈传红

void main()
{
    // 整屏基底是硬性要求（计划书 §4.2 推论 2：跟随模式下行星的角位移可达数十度），
    // 所以**不能**改成"中心亮、边缘暗"的径向渐变 —— 边缘也必须被压到白。
    // 下面的核心项只是给 bloom 一个圆心，让它从舰体位置向外晕，不是拿来替代基底的。

    vec2  d    = vNDC - uFlashCenter;
    float core = 1.0 / (1.0 + 6.0 * dot(d, d));   // 距离 0 处为 1，半屏处约 0.14

    // 加法混合（GL_ONE, GL_ONE），所以这里输出的是"要加多少"，不是最终颜色。
    // 两个默认值都远高于 bright pass 阈值（约 1.0），峰值帧必然整屏过曝并进 bloom。
    //vec3 add = vec3(uFlashBase + uFlashCore * core) * uFlash;
    vec3 add = uFlashColor * (uFlashBase + uFlashCore * core) * uFlash;
    FragColor = vec4(add, 1.0);
}