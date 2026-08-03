#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 starColor = vec3(1.0, 0.8, 0.6);	// 恒星主颜色
uniform vec3 coreColor = vec3(1.0, 0.9, 0.7);	// 恒星核心颜色
uniform float time;
uniform float intensity = 1.0;		// 恒星亮度
uniform vec3 viewPos;				// 观察者位置

float snoise(vec4 v); // 4D simplex noise 函数声明 

// 分形噪声函数
float fractalNoise(vec4 p, int octaves, float frequency, float persistence) 
{
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1.0;
    
    for (int i = 0; i < octaves; i++) 
    {
        total += snoise(p * frequency) * amplitude;
        maxAmplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    return total / maxAmplitude; // 归一化到 [-1, 1]
}

void main()
{
    vec3 dir = normalize(Normal);
    vec2 uv = TexCoords * 2.0 - 1.0; // 将 UV 映射到 [-1, 1]
    float distFromCenter = length(uv);  // 计算距离中心的距离
    
    // 湍流/米粒噪声
    float turb = fractalNoise(vec4(dir * 6.0, time * 0.4), 6, 1.0, 0.5);
    float baseTex = turb * 0.5 + 0.5; // 将噪声值映射到 [0, 1]
    // 太阳黑子
    float s = 0.3;
    float freq = 5.0;
    vec4 pos1 = vec4(dir * freq, time * 0.5);
    vec4 pos2 = vec4(dir * freq + 5000.0, time * 0.15);
    // vec4 pos1 = vec4(dir * freq, time * 0.15);
    // vec4 pos2 = vec4(dir * freq + 5000.0, time * 0.05);
    float t1 = snoise(pos1) - s;
    float t2 = snoise(pos2) - s;
    float sunspot = max(t1, 0.0) * max(t2, 0.0) * 1.1;
    baseTex = clamp(baseTex - sunspot, 0.0, 1.0);   // 混合黑子暗区

    // 亮度与脉冲
    float pulse = sin(time * 1.5) * 0.1 + 1.0;
    float centerBrightness = 1.0 - smoothstep(0.0, 1.0, distFromCenter * 1.5);
    float edgeBrightness   = exp(-distFromCenter * distFromCenter * 5.0);
    
    // 菲涅尔边缘辉光
    vec3 viewDir = normalize(viewPos - FragPos);
    float fresnel = pow(1.0 - abs(dot(normalize(Normal), viewDir)), 2.0) * 0.5;
    
    // 最终亮度
    float finalIntensity = intensity * pulse * (baseTex * 0.7 + 0.3 * centerBrightness);
    
    // 颜色混合
    vec3 finalColor = mix(starColor, coreColor, centerBrightness * 0.8);
    
    // 边缘红移
    float redShift = smoothstep(0.7, 1.0, distFromCenter) * 0.25;
    finalColor.r += redShift;
    finalColor.g -= redShift * 0.5;
    finalColor.b -= redShift * 0.8;

    // 菲涅尔发光
    finalColor += vec3(1.0, 0.6, 0.3) * fresnel * edgeBrightness;
    
    // HDR 色调映射
    
    // VER 1.0 (这个版本太暗了)
    // vec3 hdrColor = finalColor * finalIntensity;
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0)); // Reinhard 色调映射
    
    // VER 2.0 (高亮版本)
    // 更亮的映射：hdr - hdr/(1+hdr) 的变种，或直接用没有映射的原色
    // vec3 hdrColor = finalColor * finalIntensity;
    // // 保留高光的 HDR 映射
    // vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.5); // 指数映射，更亮

    // FragColor = vec4(mapped, 1.0);

    // VER 3.0 (线性 HDR 输出，后续在后处理阶段做 tone-mapping)
    vec3 hdrColor = finalColor * finalIntensity;

    FragColor = vec4(hdrColor, 1.0);
}

//================= 下面是 4D simplex noise 的实现 =================
// 4D simplex noise
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec4 mod289(vec4 x) 
{
    return x - floor(x * (1.0 / 289.0)) * 289.0; 
}

float mod289(float x) 
{
    return x - floor(x * (1.0 / 289.0)) * 289.0; 
}

vec4 permute(vec4 x) 
{
    return mod289(((x*34.0)+10.0)*x);
}

float permute(float x) 
{
    return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
{
    const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
    vec4 p,s;

    p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
    p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
    s = vec4(lessThan(p, vec4(0.0)));
    p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

    return p;
}
						
// (sqrt(5) - 1)/4 = F4, used once below
#define F4 0.309016994374947451

// 4D simplex noise
float snoise(vec4 v)
{
    const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                        0.276393202250021,  // 2 * G4
                        0.414589803375032,  // 3 * G4
                       -0.447213595499958); // -1 + 4 * G4

// First corner
    vec4 i  = floor(v + dot(v, vec4(F4)) );
    vec4 x0 = v -   i + dot(i, C.xxxx);

    // Other corners

    // Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
    vec4 i0;
    vec3 isX = step( x0.yzw, x0.xxx );
    vec3 isYZ = step( x0.zww, x0.yyz );
    //  i0.x = dot( isX, vec3( 1.0 ) );
    i0.x = isX.x + isX.y + isX.z;
    i0.yzw = 1.0 - isX;
    //  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
    i0.y += isYZ.x + isYZ.y;
    i0.zw += 1.0 - isYZ.xy;
    i0.z += isYZ.z;
    i0.w += 1.0 - isYZ.z;

    // i0 now contains the unique values 0,1,2,3 in each channel
    vec4 i3 = clamp( i0, 0.0, 1.0 );
    vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
    vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

    //  x0 = x0 - 0.0 + 0.0 * C.xxxx
    //  x1 = x0 - i1  + 1.0 * C.xxxx
    //  x2 = x0 - i2  + 2.0 * C.xxxx
    //  x3 = x0 - i3  + 3.0 * C.xxxx
    //  x4 = x0 - 1.0 + 4.0 * C.xxxx
    vec4 x1 = x0 - i1 + C.xxxx;
    vec4 x2 = x0 - i2 + C.yyyy;
    vec4 x3 = x0 - i3 + C.zzzz;
    vec4 x4 = x0 + C.wwww;

    // Permutations
    i = mod289(i); 
    float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
    vec4 j1  = permute( permute( permute( permute (
                i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
                + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
                + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
                + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

    // Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
    // 7*7*6 = 294, which is close to the ring size 17*17 = 289.
    vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

    vec4 p0 = grad4(j0,   ip);
    vec4 p1 = grad4(j1.x, ip);
    vec4 p2 = grad4(j1.y, ip);
    vec4 p3 = grad4(j1.z, ip);
    vec4 p4 = grad4(j1.w, ip);

    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    p4 *= taylorInvSqrt(dot(p4,p4));

    // Mix contributions from the five corners
    vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
    vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
    m0 = m0 * m0;
    m1 = m1 * m1;
    return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}