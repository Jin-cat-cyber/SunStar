#version 330 core
out vec4 FragColor;

uniform vec3  camPos;
uniform vec3  sunCenter;     // 恒星中心 = pointSunPositions
uniform float sunRadius;     // 体积球半径（可调接口）
uniform float time;
uniform vec2  resolution;
uniform mat4  invProjView;
uniform vec3  sunColor;      // 恒星颜色（暖橙）

// ===== 2D value noise + fbm =====
float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453123); }
float noise(vec2 p)
{
    vec2 i = floor(p); vec2 f = fract(p); vec2 u = f*f*(3.0-2.0*f);
    return mix(
        mix(hash(i+vec2(0.0,0.0)), hash(i+vec2(1.0,0.0)), u.x),
        mix(hash(i+vec2(0.0,1.0)), hash(i+vec2(1.0,1.0)), u.x),
        u.y);
}
float fbm(vec2 p)
{
    float v = 0.0; float amp = 0.5;
    for(int i=0;i<5;i++){ v += amp*noise(p); p = p*2.0+vec2(1.7,9.2); amp *= 0.5; }
    return v;
}

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
        // 球心留暗区：relDist < coreInner(恒星本体) 不透光，从外圈才起雾
        //float radial = exp(-relDist * 2.5) * smoothstep(0.18, 0.33, relDist);
        //float radial = exp(-relDist * 2.5) * smoothstep(0.33, 0.45, relDist);


        // 云絮（水平坐标 + 时间漂移）
        // vec2 uv = (p - sunCenter).xz * 0.02;

        // 云絮（球面坐标 + 时间漂移）
        vec3 q = (p - sunCenter) * 0.02;
        vec2 uv = q.xz + vec2(q.y * 1.5, 0.0);
        float n = fbm(uv + vec2(time*0.03, time*0.02));
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
