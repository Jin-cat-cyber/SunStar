#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 LocalPos;

uniform vec3 sunCenter;
uniform float time;
uniform vec3 coronaColor;
uniform float coronaIntensity = 1.0;
uniform float scale = 100.0;

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
    // 1. 缩放局部坐标，模拟 _Scale 控制密度
    vec3 position = LocalPos;
    // 2. 计算偏移 distVec 时先产生三个方向的低频噪声
    float frequency = 0.8;
    float ox = snoise(vec4(position * frequency, time));
    float oy = snoise(vec4(position + 2000.0 * frequency, time));
    float oz = snoise(vec4(position + 4000.0 * frequency, time));
    // 3. 偏转后的方向向量
    vec3 distVec = normalize(position + vec3(ox, oy, oz) * 0.25);
    // 4. 时间从内向外流逝（中心最新，边缘滞后）
    float t = time - length(position);
    // 5. 用分形噪声生成扰动，制造丝状结构
    float noiseVal = fractalNoise(vec4(distVec, t), 3, 3.0, 0.7);
    vec3 n = position + noiseVal * 0.5;
    // 6. 计算亮度：距离越远越暗
    float dist = length(n) * 1.0;
    float brightness = max((1.0 / (dist * dist) - 0.1) * 0.7, 0.0);
    float total = pow(brightness, 4.0);
    // 7. 输出，alpha也设置为total以配合 glBlendFunc(GL_SRC_ALPHA, GL_ONE)
    vec3 col = coronaColor * total * coronaIntensity;
    FragColor = vec4(col, total * coronaIntensity);

    // // 避免除0，控制最大值，输出线性 HDR（不要在这里做 tone-mapping）
    // float dist = max(length(n), 0.0001);
    // float brightness = (1.0 / (dist * dist + 1e-6) - 0.1) * 0.7;
    // brightness = clamp(brightness, 0.0, 10.0); // 防止数值爆炸
    // float total = pow(brightness, 4.0);

    // // 输出线性辐照（由后处理做 tone-mapping / gamma）
    // vec3 col = coronaColor * total * coronaIntensity;
    // float alpha = clamp(total * coronaIntensity, 0.0, 1.0);
    // FragColor = vec4(col, alpha);
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