#version 330 core
out vec4 FragColor;

uniform vec3  camPos;
uniform vec3  sunCenter;     // 恒星中心 = pointSunPositions
uniform float sunRadius;     // 体积球半径（可调接口）
uniform float time;
uniform vec2  resolution;
uniform mat4  invProjView;
uniform vec3  sunColor;      // 恒星颜色（暖橙）

// // ===== 2D value noise + fbm =====
// float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453123); }
// float noise(vec2 p)
// {
//     vec2 i = floor(p); vec2 f = fract(p); vec2 u = f*f*(3.0-2.0*f);
//     return mix(
//         mix(hash(i+vec2(0.0,0.0)), hash(i+vec2(1.0,0.0)), u.x),
//         mix(hash(i+vec2(0.0,1.0)), hash(i+vec2(1.0,1.0)), u.x),
//         u.y);
// }
// float fbm(vec2 p)
// {
//     float v = 0.0; float amp = 0.5;
//     for(int i=0;i<5;i++){ v += amp*noise(p); p = p*2.0+vec2(1.7,9.2); amp *= 0.5; }
//     return v;
// }

// ===== 3D Simplex 噪声 (无方向性,无斜纹) =====
vec3 mod289(vec3 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec4 mod289(vec4 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec4 permute(vec4 x){ return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r){ return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v);
float fbm(vec3 p);


// ray 与球交点：返回 (tEnter, tExit)，无交点时 tEnter > tExit
vec2 raySphere(vec3 ro, vec3 rd, vec3 c, float R)
{
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float c2 = dot(oc,oc) - R*R;
    float disc = b*b - c2;
    if(disc < 0.0) return vec2(1.0, 0.0);
    float sq = sqrt(disc);
    return vec2(-b - sq, -b + sq);
}

void main()
{
    // 1. 重建视线射线
    vec2 ndc = gl_FragCoord.xy / resolution * 2.0 - 1.0;
    vec4 farW = invProjView * vec4(ndc, 1.0, 1.0);
    farW /= farW.w;
    vec3 rd = normalize(farW.xyz - camPos);

    // 2. 与体积球求交
    vec2 tt = raySphere(camPos, rd, sunCenter, sunRadius);
    float tEnter = max(tt.x, 0.0);
    float tExit  = tt.y;
    if(tEnter > tExit) discard;

    int steps = 48;
    float stepLen = (tExit - tEnter) / float(steps);
    if(stepLen <= 0.0) discard;

    vec3 light = vec3(0.0);
    float transmittance = 1.0;

    for(int i=0;i<steps;i++)
    {
        float t = tEnter + (float(i)+0.5)*stepLen;
        vec3 p = camPos + rd*t;

        float distToSun = length(p - sunCenter);
        float relDist = distToSun / sunRadius;          // 0=球心, 1=球面

        // 径向衰减：球心最浓，向外衰减（淡圈 = 衰减快 + 基数低）
        //float radial = exp(-relDist * 4.5);
        // 雾变薄壳
        float shell = smoothstep(0.55, 0.75, relDist);
        float radial = exp(-relDist * 4.5) * shell;


        // 云絮（球面坐标 + 时间漂移）
        vec3 q = (p - sunCenter) * 0.02;
        float n = fbm(q + vec3(time*0.03, time*0.02, 0.0));
        n = smoothstep(0.3, 0.85, n);

        float d = radial * (0.15 + 0.20 * n);           // 淡圈：基数低

        // 纯光散射，无阴影
        float scatter = d * transmittance;
        light += sunColor * scatter * stepLen * 0.15;
        transmittance *= exp(-d * stepLen * 0.02);
    }

    // 颜色沿径向渐变：白热核心 -> 暖橙边缘
    //vec3 tint = mix(vec3(1.0, 0.95, 0.85), vec3(1.0, 0.65, 0.3), 0.6);
    // 冷白
    vec3 tint = mix(vec3(0.95, 0.98, 1.05), vec3(0.9, 0.7, 0.4), 0.4);
    light *= tint;

    float intensity = clamp(dot(light, vec3(0.2126,0.7152,0.0722)), 0.0, 1.0);
    FragColor = vec4(light, intensity);
}

// ===== 3D Simplex 噪声 =====
float snoise(vec3 v)
{
    const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    i = mod289(i);
    vec4 p = permute(permute(permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    float n_ = 0.142857142857;
    vec3 ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2,p2), dot(p3,p3)));
    p0 *= norm.x; p1 *= norm.y; p2 *= norm.z; p3 *= norm.w;

    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
}

// 3 层 fbm，直接用 3D 坐标
float fbm(vec3 p)
{
    float v = 0.0; float amp = 0.5;
    for(int i=0;i<2;i++){ v += amp*snoise(p); p = p*2.0 + vec3(1.7, 9.2, 4.1); amp *= 0.5; }
    return v * 0.5 + 0.5;    // 归一化到 0~1
}