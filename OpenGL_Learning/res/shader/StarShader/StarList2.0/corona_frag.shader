#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


uniform vec3 sunCenter;		// 恒星中心位置
uniform float time;
uniform vec3 coronaColor;
uniform float coronaIntensity = 1.0;		// 恒星亮度
uniform float coronaRadius = 204.0;

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
    // ver1.0
    // // 计算片段位置相对于恒星中心的向量
    // vec3 pos = FragPos - sunCenter;

    // // 扰动方向
    // float frequency = 0.8;
    // float ox = snoise(vec4(pos * frequency, time));
    // float oy = snoise(vec4((pos + 2000.0) * frequency, time));
    // float oz = snoise(vec4((pos + 4000.0) * frequency, time));
    // vec3 distance = normalize(pos + vec3(ox, oy, oz) * 0.25); // 添加扰动
    // // 时间从内向外流逝
    // float t = time - length(pos);
    // // 分型噪声扭曲
    // float noiseVal = fractalNoise(vec4(distance, t), 3, 3.0, 0.7);
    // vec3 n = pos + vec3(noiseVal * 0.5);
    // // 亮度衰减
    // float dist = length(n) * 1.0;
    // float brightness = (1.0 / (dist * dist) - 0.1) * 0.7;
    // brightness = max(brightness, 0.0);
    // float total = pow(brightness, 4.0);
    // // 最终颜色
    // vec3 coronaColor = vec3(1.0, 0.9, 0.6);
    // vec3 finalColor = coronaColor * total * coronaIntensity * 2.5;
    // //加法混合下不需要alpha， 设为1.0即可
    // FragColor = vec4(finalColor, 1.0);

    // ver2.0
    //  vec3 pos = FragPos - sunCenter;
    // float distFromCenter = length(pos);
    // float relDist = distFromCenter / coronaRadius;   // 0~1范围
    // // 放大频率和扰动幅度以适配模型尺寸
    // float scale = coronaRadius;
    // float frequency = 0.8 * scale;
    // float ox = snoise(vec4(pos * frequency, time));
    // float oy = snoise(vec4((pos + vec3(2000.0)) * frequency, time));
    // float oz = snoise(vec4((pos + vec3(4000.0)) * frequency, time));
    // vec3 distVec = normalize(pos + vec3(ox, oy, oz) * (0.25 * scale));
    // float t = time - distFromCenter * 0.1;
    // float noiseVal = fractalNoise(vec4(distVec, t), 3, 3.0 * scale, 0.7);
    // vec3 n = pos + vec3(noiseVal * 0.5 * scale);
    // // 用相对距离计算亮度（修复-0.1）
    // float finalRelDist = length(n) / coronaRadius;
    // float brightness = (1.0 / (finalRelDist * finalRelDist) - 0.1) * 0.7;
    // brightness = max(brightness, 0.0);
    // float total = pow(brightness, 4.0);
    // vec3 coronaColor = vec3(1.0, 0.9, 0.6);
    // vec3 finalColor = coronaColor * total * coronaIntensity * 2.5;
    // FragColor = vec4(finalColor, 1.0);

        vec3 pos = FragPos - sunCenter;
    float distFromCenter = length(pos);
    vec3 dir = normalize(pos);
    // ---------- 径向因子：只在日冕壳表面附近亮 ----------
    float relDist = distFromCenter / coronaRadius;
    // 让亮度在 0.85~1.0 之间达到峰值（即日冕外缘），内侧和外侧都衰减
    float radial = smoothstep(0.85, 0.95, relDist) * (1.0 - smoothstep(0.95, 1.05, relDist));
    // ---------- 方向噪声生成流束 ----------
    // 基础频率较低，产生大尺度结构
    float baseNoise = fractalNoise(vec4(dir * 5.0, time * 0.2), 4, 2.0, 0.6);
    // 加上高频细节，使丝更锐利
    float detailNoise = snoise(vec4(dir * 15.0, time * 0.3)) * 0.5;
    float noiseVal = baseNoise + detailNoise;
    noiseVal = noiseVal * 0.5 + 0.5;   // 映射到 0~1
    // 阈值：只让噪声值超过 0.6 的区域亮，形成丝状
    float threshold = 0.6;
    float streak = smoothstep(threshold, threshold + 0.2, noiseVal);
    // ---------- 最终亮度 ----------
    float brightness = streak * radial * coronaIntensity * 2.0;
    // 颜色
    //vec3 coronaColor = vec3(1.0, 0.9, 0.6);
    vec3 finalColor = coronaColor * brightness;
    FragColor = vec4(finalColor, 1.0);
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