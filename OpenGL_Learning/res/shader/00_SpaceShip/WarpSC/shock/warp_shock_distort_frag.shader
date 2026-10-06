#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScene;      // hdrColorBuffer：场景 + 光柱 + 波带
uniform sampler2D uDepth;      // hdrDepthTex：窗口深度（非线性）
uniform mat4  uVP;             // 投影 × 视图（把世界位移投到屏幕上取差分）
uniform mat4  uInvVP;          // 其逆（把像素反算成世界坐标）
uniform vec3  uCenter;         // 环心（世界）—— 去程是舰体中心，抵达是逻辑面原点
uniform vec3  uAxisDir;        // 环所在平面的法线 = 舰体纵轴（世界，单位向量）
uniform float uRadius;         // 当前波前半径（世界单位）
uniform float uProfileW;       // 单峰剖面半宽（世界单位）
uniform float uDistortPx;      // 偏移幅度（像素 @1080p）
uniform float uPush;           // +1 向外推、-1 向内吸
uniform vec2  uScreenSize;     // 分辨率（像素）

void main()
{
    float d = texture(uDepth, TexCoords).r;

    //  深度 = 1 表示那个像素没有几何（星空/背景），反算不出世界坐标 —— 原样输出。
    if (d >= 1.0)
    {
        FragColor = texture(uScene, TexCoords);
        return;
    }

    //  反算世界坐标：深度存的是【窗口深度】，先还原成 NDC 再乘逆 VP。
    vec4 world = uInvVP * vec4(TexCoords * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
    world /= world.w;

    //  该点到【那圈圆环】的距离（三维，含面外分量）。环躺在法线为 uAxisDir 的平面里。
    vec3  v = world.xyz - uCenter;
    float h = dot(v, uAxisDir);          // 面外分量
    vec3  q = v - h * uAxisDir;          // 面内分量
    //  变量名不能叫 d —— 上面已经用 d 装了窗口深度（C1038）。
    float dp = length(q);
    if (dp < 1e-4) { FragColor = texture(uScene, TexCoords); return; }   // 落在轴上，面内径向无定义
    vec3  radial  = q / dp;
    vec3  nearest = uCenter + radial * uRadius + h * uAxisDir;   // 环上离该像素最近的点
    vec3  delta   = world.xyz - nearest;
    float eps     = length(delta);                               // 这就是"离波前多远"
    if (eps < 1e-4) { FragColor = texture(uScene, TexCoords); return; }

    //  单峰剖面。
    float x = eps / uProfileW;
    float w = exp(-x * x);

    //  屏幕空间的径向方向：把"从环上最近点指向该像素"的世界位移投到屏幕上取差分。
    vec4 c0 = uVP * vec4(world.xyz, 1.0);
    vec4 c1 = uVP * vec4(world.xyz + (delta / eps) * max(uRadius, 1.0), 1.0);
    vec2 s0 = (c0.xy / c0.w) * 0.5 + 0.5;
    vec2 s1 = (c1.xy / c1.w) * 0.5 + 0.5;
    vec2 dirPx = (s1 - s0) * uScreenSize;
    //  轴大致沿视线时这个差分接近零，方向无意义 —— 那种像素本来也不该被推。
    if (length(dirPx) < 1e-6) { FragColor = texture(uScene, TexCoords); return; }
    vec2 dir = normalize(dirPx);

    //  uDistortPx 是"@1080p"的值，按实际分辨率等比缩放。
    float scale  = uScreenSize.y / 1080.0;
    vec2  offset = dir * (uPush * uDistortPx * scale * w) / uScreenSize;

    FragColor = texture(uScene, TexCoords + offset);
}