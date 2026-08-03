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

// 生成随机数的函数
float rand(vec2 co) 
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// 生成噪声的函数
float noise(vec2 p)
{
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u * u * (3.0 - 2.0 * u);

	float res = mix(
		mix(rand(ip), rand(ip + vec2(1.0, 0.0)), u.x),
		mix(rand(ip + vec2(0.0, 1.0)), rand(ip + vec2(1.0, 1.0)), u.x),
		u.y);
		return res * res; // 增加对比度
}

// 分形布朗运动（FBM）函数
float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // 基于UV坐标计算距离中心的位置
    vec2 uv = TexCoords * 2.0 - 1.0;
    float distFromCenter = length(uv);
    
    // 1. 恒星表面湍流效果
    float turbulence = fbm(uv * 5.0 + time * 0.5) * 0.3;
    
    // 2. 恒星耀斑（随机热点）
    float hotspots = 0.0;
    for (int i = 0; i < 8; i++)
    {
        float angle = float(i) * 0.785; // 45度间隔
        vec2 hotspotPos = vec2(cos(angle), sin(angle)) * 0.4;
        float hotspotDist = distance(uv, hotspotPos);
        float hotspotTime = time * (0.5 + rand(vec2(float(i) * 0.1)));
        float hotspotIntensity = sin(hotspotTime) * 0.5 + 0.5;
        hotspots += exp(-hotspotDist * 20.0) * hotspotIntensity * 0.3;
    }
    
    // 3. 太阳黑子效果
    float sunspots = 0.0;
    for (int i = 0; i < 6; i++)
    {
        // 固定位置的黑子（更简单）
        // float angle = float(i) * 1.57; // 90度间隔
        // vec2 spotPos = vec2(cos(angle), sin(angle)) * 0.6;
        // float spotDist = distance(uv, spotPos);
        // float spotSize = 0.1 + rand(vec2(float(i) * 0.2)) * 0.1;
        // sunspots += 1.0 - smoothstep(spotSize * 0.8, spotSize, spotDist);

        // 太阳黑子动态效果
        // 使黑子位置随时间缓慢移动
        float moveSpeed = 0.08 + rand(vec2(float(i) * 0.3)) * 0.03;
        float angle = float(i) * 1.256 + time * moveSpeed; // 72度间隔
        float radius = 0.6 + rand(vec2(float(i) * 0.5)) * 0.15; // 随机半径
        
        vec2 spotPos = vec2(cos(angle), sin(angle)) * radius;
        float spotDist = distance(uv, spotPos);
        
        // 动态变化的黑子大小
        float spotSize = 0.05 + rand(vec2(float(i) * 0.7)) * 0.04;
        float sizeVariation = 0.01 * sin(time * (0.3 + rand(vec2(float(i) * 0.2))));
        spotSize += sizeVariation;
        
        // 黑子形状 - 使用更清晰的定义
        float spot = 1.0 - smoothstep(spotSize * 0.2, spotSize * 0.8, spotDist);
        
        // 黑子核心更暗
        float spotCore = 1.0 - smoothstep(0.0, spotSize * 0.3, spotDist);
        spot = mix(spot, spotCore * 1.3, 0.8); // 核心区域更突出
        
        // 黑子周围的较暗区域（本影）
        float penumbra = 1.0 - smoothstep(spotSize * 0.8, spotSize * 1.2, spotDist);
        spot += penumbra * 0.1;
        
        sunspots += spot * (1.0 + rand(vec2(float(i) * 0.4)) * 0.4);
    }
    sunspots = clamp(sunspots, 0.0, 1.2);
    
    // 4. 核心亮度和边缘衰减
    float centerBrightness = 1.0 - smoothstep(0.0, 1.0, distFromCenter * 1.5);
    float edgeBrightness = exp(-distFromCenter * distFromCenter * 5.0);
    
    // 5. 动态脉动
    float pulse = sin(time * 1.5) * 0.1 + 1.0;
    
    // 6. 基于视角的菲涅尔效应（边缘发光）
    vec3 viewDir = normalize(viewPos - FragPos);
    float fresnel = 1.0 - dot(normalize(Normal), viewDir);
    fresnel = pow(fresnel, 2.0) * 0.5;
    
    // 7. 组合所有效果
    float finalIntensity = intensity * pulse * (1.0 + turbulence + hotspots - sunspots * 0.3);
    
    // 8. 颜色计算
    vec3 finalColor = mix(starColor, coreColor, centerBrightness * 0.8);

    // 黑子区域的颜色 - 使其更暗、偏红棕色
    //vec3 sunspotColor = vec3(0.6, 0.4, 0.2); // 暗红棕色
    vec3 sunspotColor = vec3(0.4, 0.25, 0.15); // 更深的红棕色
    finalColor = mix(finalColor, sunspotColor, sunspots * 0.5);
    
    
    // 添加边缘的红色调（像真实的太阳）
    float redShift = smoothstep(0.7, 1.0, distFromCenter) * 0.3;
    finalColor.r += redShift;
    finalColor.g -= redShift * 0.5;
    finalColor.b -= redShift * 0.8;
    
    // 应用菲涅尔边缘发光
    finalColor += vec3(1.0, 0.6, 0.3) * fresnel * edgeBrightness;
    
    // 9. 最终输出（HDR风格）
    vec3 hdrColor = finalColor * finalIntensity;
    
    // 10. 色调映射（Reinhard）
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    
    FragColor = vec4(mapped, 1.0);
}