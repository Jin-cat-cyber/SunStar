#version 330 core
out vec4 FragColor;

uniform vec3  camPos;
uniform vec3  ringCenter;
uniform float ringInner;
uniform float ringOuter;
uniform float ringHalfHeight;
uniform float planetRadius;        // 火星世界半径
uniform vec3  ringColor;
uniform float time;
uniform vec2  resolution;
uniform mat4  invProjView;

// ===== 光照 + 阴影 =====
uniform vec3        sunPos;
uniform vec3        sunColor;
uniform samplerCube depthMap;
uniform float       far_plane;

// ===== 2D value noise + fbm =====
float hash(vec2 p)
{ 
    return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453123); 
}

// 2D value noise
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(hash(i + vec2(0.0, 0.0)), hash(i+vec2(1.0, 0.0)), u.x),
        mix(hash(i + vec2(0.0, 1.0)), hash(i+vec2(1.0, 1.0)), u.x),
        u.y);
}

// 2D 分形布朗运动
float fbm(vec2 p)
{
    float value = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; i++)
    { 
        value += amp * noise(p); 
        p = p * 2.0 + vec2(1.7,9.2); 
        amp *= 0.5; 
    }
    return value;
}

// 基础阴影
float ShadowCalculation(vec3 p)
{
    vec3 fragToLight = p - sunPos;
    float currentDepth = length(fragToLight);
    float closestDepth = texture(depthMap, fragToLight).r * far_plane;
    float bias = 21.0;
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}

// slab 求射线与 AABB 交点，返回 (tEnter, tExit)
vec2 rayBox(vec3 ro, vec3 rd, vec3 c, vec3 halfExt)
{
    vec3 mn = c - halfExt;
    vec3 mx = c + halfExt;

    vec3 inv = 1.0 / rd;

    vec3 t0 = (mn - ro) * inv;
    vec3 t1 = (mx - ro) * inv;

    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float tEnter = max(max(tmin.x, tmin.y), tmin.z);
    float tExit  = min(min(tmax.x, tmax.y), tmax.z);

    return vec2(tEnter, tExit);
}

// 求射线与球的近交点（相机在球外时），不相交返回 -1
float raySphere(vec3 ro, vec3 rd, vec3 c, float R)
{
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float c2 = dot(oc, oc) - R * R;
    float disc = b * b - c2;

    if (disc < 0.0) 
        return -1.0;
    
    return -b - sqrt(disc);
}

void main()
{

    vec3 sunDir = normalize(sunPos - ringCenter);


    // 1. 重建真实视线射线（每个像素，方向永远向前）
    vec2 ndc = gl_FragCoord.xy / resolution * 2.0 - 1.0;
    vec4 farW = invProjView * vec4(ndc, 1.0, 1.0);
    farW /= farW.w;
    vec3 rd = normalize(farW.xyz - camPos);

    // 2. 与环体积盒求交
    vec3 halfExt = vec3(ringOuter, ringHalfHeight, ringOuter);
    vec2 tb = rayBox(camPos, rd, ringCenter, halfExt);
    float tEnter = max(tb.x, 0.0);
    float tExit  = tb.y;
    if (tEnter > tExit) discard;

    // 3. 火星遮挡：march 到火星前表面就停
    float tPlanet = raySphere(camPos, rd, ringCenter, planetRadius);
    float tEnd = tExit;
    if (tPlanet > 0.0) tEnd = min(tEnd, tPlanet);

    int steps = 64;
    
    float stepLen = (tEnd - tEnter) / float(steps);
    
    if (stepLen <= 0.0) discard;

    vec3 light = vec3(0.0);
    float transmittance = 1.0;

    for (int i = 0; i < steps; i++)
    {
        float t = tEnter + (float(i) + 0.5) * stepLen;
        vec3 p = camPos + rd * t;

        vec2 offset = p.xz - ringCenter.xz;
        float r = length(offset);
        float h = p.y - ringCenter.y;

        if (r > ringInner && r < ringOuter && abs(h) < ringHalfHeight)
        {
            float relDist = (r - ringInner) / (ringOuter - ringInner);
            float radial = smoothstep(0.0, 0.15, relDist) * (1.0 - smoothstep(0.6, 1.0, relDist));
            float vertical = exp(-(h*h) / (ringHalfHeight * ringHalfHeight) * 2.0);
            
            vec2 uv = offset * 0.045;
            
            float n = fbm(uv + vec2(time * 0.03, time * 0.02));
            n = smoothstep(0.3, 0.8, n);
            float d = radial * vertical * (0.5 + 0.7 * n);

            float shadow = ShadowCalculation(p);

            // 当前采样点 p 相对环中心的向外方向
            vec3 outward = normalize(p - ringCenter);
            // 亮暗面系数：朝太阳的一面 lit 高，背光面 lit 低
            float lit = 0.4 + 0.6 * max(dot(outward, sunDir), 0.0);

            // 颜色插值
            vec3 tint = mix(vec3(1.0, 0.75, 0.5), vec3(1.0, 0.95, 0.8), smoothstep(0.15, 0.6, relDist));

            float scatter = d * (1.0 - shadow) * transmittance * lit;
            light += sunColor * tint * scatter * stepLen * 0.22;
            // Beer-Lambert 定律：光在介质中传播时会被吸收，吸收量与介质厚度成指数关系
            transmittance *= exp(-d * stepLen * 0.025);
        }
    }

    float intensity = clamp(dot(light, vec3(0.2126, 0.7152, 0.0722)), 0.0, 1.0);
    FragColor = vec4(light, intensity);
}
