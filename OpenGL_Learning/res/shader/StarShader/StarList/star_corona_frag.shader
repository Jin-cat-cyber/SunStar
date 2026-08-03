// // VER 1.0
// #version 330 core
// out vec4 FragColor;

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 starColor = vec3(1.0, 0.7, 0.5);
// uniform float time;
// uniform float coronaIntensity = 0.8;
// uniform vec3 viewPos;

// // 旋转矩阵（用于创建日冕射线）
// mat2 rotate2D(float angle)
// {
//     float s = sin(angle);
//     float c = cos(angle);
//     return mat2(c, -s, s, c);
// }

// // 射线生成函数
// float createRay(vec2 uv, float angle, float width)
// {
//     uv = rotate2D(angle) * uv;
//     float ray = smoothstep(0.0, width, uv.x) * smoothstep(width * 2.0, width, uv.x);
//     ray *= exp(-abs(uv.y) * 10.0);
//     return ray;

// }

// void main()
// {
//     // 基于UV坐标计算距离中心的位置
//     vec2 uv = TexCoords * 2.0 - 1.0;
//     float distFromCenter = length(uv);   

//     // 1. 基本日冕衰减（从中心向外）
//     float baseAttenuation = 1.0 - smoothstep(0.3, 0.8, distFromCenter);
    
//     // 2. 创建日冕射线
//     float rays = 0.0;
//     int rayCount = 12; // 射线数量
    
//     for (int i = 0; i < rayCount; i++)
//     {
//         float angle = float(i) * (6.283185 / float(rayCount));
//         float rayTimeOffset = float(i) * 0.5;
        
//         // 动态变化的射线宽度
//         float rayWidth = 0.03 + sin(time * 2.0 + rayTimeOffset) * 0.01;
        
//         // 计算单个射线
//         float ray = createRay(uv, angle + time * 0.3, rayWidth);
        
//         // 添加射线脉动
//         float rayPulse = sin(time * 4.0 + float(i) * 0.7) * 0.3 + 0.7;
//         rays += ray * rayPulse;
//     }
    
//     // 3. 日冕波纹效果
//     float coronaWaves = 0.0;
//     for (int i = 0; i < 3; i++)
//     {
//         float waveFreq = float(i + 1) * 3.0;
//         float waveSpeed = float(i + 1) * 1.5;
//         coronaWaves += sin(distFromCenter * waveFreq - time * waveSpeed) * 0.1;
//     }
    
//     // 4. 基于视角的透明度变化
//     vec3 viewDir = normalize(viewPos - FragPos);
//     float viewDot = dot(normalize(Normal), viewDir);
//     float edgeAlpha = pow(1.0 - abs(viewDot), 2.0);
    
//     // 5. 组合所有效果
//     float coronaEffect = baseAttenuation * (1.0 + rays * 0.8 + coronaWaves);
    
//     // 6. 颜色渐变（从内到外）
//     vec3 innerColor = vec3(1.0, 0.9, 0.6);  // 内部：黄色
//     vec3 outerColor = vec3(1.0, 0.4, 0.2);  // 外部：红色
    
//     float colorMix = smoothstep(0.2, 0.7, distFromCenter);
//     vec3 coronaColor = mix(innerColor, outerColor, colorMix);
    

//      // 7. 合成强度与透明度
//     float intensity = baseAttenuation * (1.0 + rays * 0.9 + coronaWaves * 0.5);
//     intensity *= coronaIntensity;
//     intensity = max(0.0, intensity);

//     float alpha = clamp((baseAttenuation * 0.35 + rays * 0.45) * edgeAlpha, 0.0, 0.85);

//     // 8. 最终色彩（对亮度做轻微色调映射，避免过曝）
//     vec3 finalColor = coronaColor * intensity;
//     finalColor = finalColor / (finalColor + vec3(1.0)); // 简单 Reinhard 映射

//     FragColor = vec4(finalColor, alpha);
//     // 7. 最终颜色和透明度
//     // vec3 finalColor = coronaColor * coronaIntensity * coronaEffect;
//     // float alpha = baseAttenuation * edgeAlpha * 0.4; // 控制整体透明度
    
//     // // 根据射线强度增加不透明度
//     // alpha += rays * 0.3;
//     // alpha = clamp(alpha, 0.0, 0.8);
    
//     // FragColor = vec4(finalColor, alpha);
// }

// VER 2.0
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 starColor = vec3(1.0, 0.7, 0.5);
uniform float time;
uniform float coronaIntensity = 1.0; // 增加默认强度
uniform vec3 viewPos;

void main()
{
    // 基于UV坐标计算距离中心的位置
    vec2 uv = TexCoords * 2.0 - 1.0;
    float distFromCenter = length(uv);   
    
    // 移除discard，改用透明度控制
    // 1. 基本日冕衰减（从中心向外）
    float baseAttenuation = 1.0 - smoothstep(0.3, 1.0, distFromCenter);
    
    // 2. 简单的边缘光晕
    float edgeGlow = exp(-distFromCenter * distFromCenter * 4.0) * 0.8;
    
    // 3. 添加一些动态效果
    float pulse = sin(time * 2.0) * 0.2 + 1.0;
    
    // 4. 基于视角的透明度
    vec3 viewDir = normalize(viewPos - FragPos);
    float viewDot = dot(normalize(Normal), viewDir);
    float edgeAlpha = pow(1.0 - abs(viewDot), 3.0) * 0.5;
    
    // 5. 组合效果
    float intensity = baseAttenuation * edgeGlow * pulse * coronaIntensity;
    
    // 6. 颜色渐变
    vec3 innerColor = vec3(1.0, 0.9, 0.6);
    vec3 outerColor = vec3(1.0, 0.4, 0.2);
    float colorMix = smoothstep(0.3, 0.9, distFromCenter);
    vec3 coronaColor = mix(innerColor, outerColor, colorMix);
    
    // 7. 最终颜色和透明度
    vec3 finalColor = coronaColor * intensity;
    float alpha = intensity * 0.6 + edgeAlpha * 0.3; // 增加整体透明度
    
    FragColor = vec4(finalColor, alpha * 0.8); // 确保足够透明可见
}


// #version 330 core
// out vec4 FragColor;

// void main()
// {
//     // 输出一个固定的红色，透明度为0.5
//     FragColor = vec4(1.0, 0.0, 0.0, 0.5);
// }
