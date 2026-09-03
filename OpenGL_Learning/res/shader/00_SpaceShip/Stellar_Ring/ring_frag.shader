#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform vec3 planetCenter;
uniform float ringInner;
uniform float ringOuter;
uniform vec3 ringColor;
uniform float time;
uniform bool shadows;

// ===== 光照 + 阴影 =====
uniform vec3        sunPos;
uniform samplerCube depthMap;
uniform float       far_plane;

// ===== 2D value noise + fbm（生成尘埃浓淡纹理）=====
float hash(vec2 p);
float noise(vec2 p);
float fbm(vec2 p);

float ShadowCalculation(vec3 fragPos);



void main()
{
    // 片元相对行星中心的位置（XZ 平面）
    vec2 offset = WorldPos.xz - planetCenter.xz;
    float r = length(offset);

    // 归一化径向位置：内边缘 0 → 外边缘 1
    float relDist = (r - ringInner) / (ringOuter - ringInner);

    // 径向包络：内外边缘渐隐，中间密
    float innerEdge = smoothstep(0.0, 0.18, relDist);        // 内缘 18% 宽度渐入
    float outerEdge = 1.0 - smoothstep(0.55, 1.0, relDist);  // 外缘 45% 宽度渐隐，拖出弥散尾巴
    float radial = innerEdge * outerEdge;
    //float radial = smoothstep(0.0, 0.06, relDist) * (1.0 - smoothstep(0.85, 1.0, relDist));

    // 尘埃浓淡：fbm 噪声，随时间缓慢漂移
    vec2 uv = offset * 0.15;                       // 噪声频率（调小=更疏，调大=更碎）
    float n = fbm(uv + vec2(time * 0.03, time * 0.02));
    n = smoothstep(0.3, 0.8, n);                  // 压缩再拉伸，产生明显团块

    // 密度 = 径向包络 × 噪声调制
    //float density = radial * (0.35 + 0.65 * n);
    float density = radial * (0.25 + 0.75 * n);

    // 向阳面光照
    vec3 sunDir = normalize(sunPos - planetCenter);
    vec2 sunDirXZ = normalize(vec2(sunDir.x, sunDir.z));    //太阳在环带上的投影防向
    vec2 dirXZ = normalize(offset);                         //片元在环带上的径向防向
    float sunFacing = dot(dirXZ, sunDirXZ);                 // -1~1
    float dayLight = 0.35 +0.65 * (0.5 + 0.5 * sunFacing);  // 向阳 1.0， 背阳 0.35

    // ===== 接收阴影 =====
    float shadow = shadows ? ShadowCalculation(WorldPos) : 0.0;

    float lighting = dayLight * (1.0 - shadow);

    FragColor = vec4(ringColor * lighting, density * 0.85);
}




// ===== PCF 阴影（复用场景 depthCubeMap，采样点 20 个）=====
float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - sunPos;
    float currentDepth = length(fragToLight);
    float closestDepth = texture(depthMap, fragToLight).r * far_plane;

    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    float shadow = 0.0;
    float bias = 40;   // 尘埃环偏置，先和小行星带一致，实调
    float diskRadius = (1.0 + (currentDepth / far_plane)) / 25.0;

    for (int i = 0; i < 20; ++i)
    {
        float d = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r * far_plane;
        if (currentDepth - bias > d)
            shadow += 1.0;
    }
    shadow /= 20.0;
    return shadow;
}


// ===== 2D value noise + fbm（生成尘埃浓淡纹理）=====
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);   // 平滑过渡
    return mix(
        mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x),
        mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
        u.y
    );
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; i++)
    {
        value += amplitude * noise(p);
        p = p * 2.0 + vec2(1.7, 9.2);   // 每层偏移，避免重叠
        amplitude *= 0.5;
    }
    return value;   // 0~1
}
