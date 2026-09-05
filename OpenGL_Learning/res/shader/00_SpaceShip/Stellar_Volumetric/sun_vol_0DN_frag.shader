#version 330 core
out vec4 FragColor;

uniform vec3  camPos;
uniform vec3  sunCenter;
uniform float sunRadius;
uniform float time;
uniform vec2  resolution;
uniform mat4  invProjView;
uniform vec3  sunColor;

// ===== 3D Simplex 噪声（仅用于"可选超轻噪声"，默认几乎不用）=====
vec3 mod289(vec3 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec4 mod289(vec4 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec4 permute(vec4 x){ return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r){ return 1.79284291400159 - 0.85373472095314 * r; }

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

// ===== simplex — 单层采样（只取第一层，便宜）=====
float simpleNoise(vec3 p)
{
    return snoise(p) * 0.5 + 0.5;   // 归一化 0~1
}

// ray 与球交点
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
        float relDist = distToSun / sunRadius;

        // 径向衰减 × 薄壳（纯渐变，无噪声主分量）
        float shell = smoothstep(0.45, 0.85, relDist);
        float radial = exp(-relDist * 2.8) * shell;

        // ===== 可选：超轻噪声（默认几乎为0，想要淡淡的不均匀就解除注释）=====
        float n = simpleNoise((p - sunCenter) * 0.05 + vec3(time*0.03, time*0.02, 0.0));

        // 纯渐变（推荐）
        //float d = radial;
        // 或带一点点呼吸感的极弱噪声（把下面这行解注释，并注释掉上面一行）
        float d = radial * (0.95 + 0.05 * n);   // 0.05 = 超轻，几乎不影响纯渐变

        float scatter = d * transmittance;
        light += sunColor * scatter * stepLen * 0.15;
        transmittance *= exp(-d * stepLen * 0.02);
    }

    // 颜色沿径向渐变
    vec3 tint = mix(vec3(0.95, 0.98, 1.05), vec3(0.9, 0.7, 0.4), 0.4);
    light *= tint;

    float intensity = clamp(dot(light, vec3(0.2126,0.7152,0.0722)), 0.0, 1.0);
    FragColor = vec4(light, intensity);
}
