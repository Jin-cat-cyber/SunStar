

// #version 330 core
// out vec4 FragColor;

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 starColor = vec3(1.0, 0.9, 0.7);
// uniform float time;
// uniform float glowIntensity = 1.0;  // 增加默认强度
// uniform vec3 viewPos;

// // 简单的高斯光晕函数
// float gaussianGlow(float dist, float size)
// {
//     return exp(-dist * dist * size);
// }

// void main()
// {
//     // 方法1：使用法线计算光晕（更可靠）
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 norm = normalize(Normal);
    
//     // 计算面向视角的程度
//     float facing = max(0.0, dot(norm, viewDir));
    
//     // 方法2：同时使用UV和法线
//     // 基于UV计算径向距离（注意：球体UV不是完美的圆）
//     vec2 uv = TexCoords * 2.0 - 1.0;
//     float uvDist = length(uv);
    
//     // 结合两种方法
//     float dist = mix(uvDist, 1.0 - facing, 0.5);
    
//     // 多层光晕
//     float glow1 = gaussianGlow(dist, 0.5);
//     float glow2 = gaussianGlow(dist, 1.0);
//     float glow3 = gaussianGlow(dist, 2.0);
    
//     // 脉动效果
//     float pulse = 1.0 + sin(time * 1.5) * 0.1;
    
//     // 组合光晕
//     float totalGlow = (glow1 * 0.5 + glow2 * 0.3 + glow3 * 0.2) * glowIntensity * pulse;
    
//     // 边缘增强
//     float edge = smoothstep(0.7, 1.0, dist);
//     float edgeGlow = pow(edge, 2.0) * 0.3;
    
//     totalGlow = totalGlow * (1.0 + edgeGlow);
    
//     // 简单闪烁
//     float flicker = 0.9 + 0.1 * sin(time * 4.0 + FragPos.x * 10.0);
    
//     // 颜色 - 保持简单
//     vec3 glowColor = starColor;
    
//     // 添加一点边缘色散
//     if (dist > 0.8)
//     {
//         float t = smoothstep(0.8, 1.0, dist);
//         glowColor = mix(glowColor, vec3(0.8, 0.85, 1.0), t * 0.3);
//     }
    
//     // 透明度计算 - 简化
//     float alpha = totalGlow * 0.6;  // 固定基础透明度
//     alpha = min(alpha, 0.8);  // 上限0.8
    
//     // 从侧面看时增加透明度
//     float sideAlpha = 1.0 - abs(dot(norm, viewDir));
//     alpha = mix(alpha, alpha * 1.5, sideAlpha);
    
//     // 确保最小透明度
//     alpha = max(alpha, 0.3);
    
//     // 最终颜色
//     vec3 finalColor = glowColor * totalGlow * flicker;
    
//     //FragColor = vec4(finalColor, alpha);
//     FragColor = vec4(1.0, 0.5, 0.2, 0.7);
    
//     // 调试：查看alpha值
//     // if (alpha < 0.01) discard;
// }

// // VER 3.0
// #version 330 core
// out vec4 FragColor;

// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 starColor;
// uniform float time;
// uniform float glowIntensity;
// uniform vec3 viewPos;

// void main()
// {
//     // 简单的基于UV的圆形光晕
//     vec2 uv = TexCoords * 2.0 - 1.0; // 将UV从[0,1]映射到[-1,1]
//     float dist = length(uv);
    
//     // 高斯函数，使得中心亮，边缘暗
//     float glow = exp(-dist * dist * 4.0);
    
//     // 脉动
//     float pulse = 1.0 + sin(time * 2.0) * 0.1;
    
//     // 颜色
//     vec3 color = starColor * glow * pulse * glowIntensity;
    
//     // 透明度：中心不透明，边缘透明
//     float alpha = glow * 0.8; // 调整这个值来改变整体透明度
    
//     FragColor = vec4(color, alpha);
// }

// VER 4.0
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;  // 可选，如果需要纹理坐标的话

uniform vec3 starColor;
uniform float time;
uniform float glowIntensity;
uniform vec3 viewPos;

void main()
{
    // 极简方法：基于法线朝向的简单辉光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 norm = normalize(Normal);
    
    // 计算朝向视角的程度
    float facing = dot(norm, viewDir);
    facing = max(0.0, facing);
    
    // 简单的辉光计算
    float glow = pow(facing, 2.0) * glowIntensity;
    
    // 简单的脉动
    float pulse = 0.9 + 0.1 * sin(time * 2.0);
    
    // 添加一些边缘光晕效果
    float edge = 1.0 - facing;
    float edgeGlow = pow(edge, 3.0) * 0.4;
    
    // 总辉光 = 面向视角的辉光 + 边缘辉光
    float totalGlow = glow + edgeGlow;
    
    // 颜色计算
    vec3 color = starColor * totalGlow * pulse;
    
    // 透明度 - 中心更透明，边缘更不透明
    float alpha = totalGlow * 0.6;
    
    // 确保最小可见度
    alpha = max(alpha, 0.3);
    
    // 如果辉光太弱，直接丢弃（可选）
    if (totalGlow < 0.02) 
    {
        alpha = 0.0;
    }
    else
    {
        alpha = max(alpha, 0.15); // 最小透明度降低
        alpha = min(alpha, 0.6);  // 最大透明度也降低
    }
    
    // 添加一点颜色变化，使辉光更自然
    // 边缘略微偏橙红色
    float colorShift = smoothstep(0.3, 0.8, edge);
    color = mix(color, color * vec3(1.0, 0.85, 0.7), colorShift * 0.3);

    FragColor = vec4(color, alpha);
}


