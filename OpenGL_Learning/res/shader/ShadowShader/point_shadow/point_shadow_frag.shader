#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float lightIntensity;

uniform float far_plane;
uniform bool shadows;

float ShadowCalculation(vec3 fragPos)
{
    vec3 templightDir = normalize(lightPos - fs_in.FragPos);
    // 获取片段位置和光源位置之间的向量
    vec3 fragToLight = fragPos - lightPos;
    // 使用该片段来点亮向量，以便从深度图中采样    
    float closestDepth = texture(depthMap, fragToLight).r;
    // 它目前处于[0,1]之间的线性范围内，让我们将其重新转换回原始深度值
    closestDepth *= far_plane;
    // 现在获取当前的线性深度，即片段与光源位置之间的距离
    float currentDepth = length(fragToLight);
    // 测试阴影
    float bias = 0.005; // 我们使用了大得多的偏差，因为深度现在处于[近平面，远平面]范围内
     
    // 直接比较当前深度和最接近的深度值来确定片段是否在阴影中
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // 将closestDepth显示为调试信息（以可视化深度立方体贴图）
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    


    // // PCF VER 1.0 (逐个采样，但由于近平面开销大)
    // // -----------------------------
    // float shadow  = 0.0;
    // float bias1   = max(0.05 * (1.0 - dot(fs_in.Normal, templightDir)), 0.005);
    // float samples = 4.0;
    // float offset  = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
    //     for(float y = -offset; y < offset; y += offset / (samples * 0.5))
    //     {
    //         for (float z = -offset; z < offset; z += offset / (samples * 0.5))
    //         {
    //             float pcfDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r;
    //             pcfDepth *= far_plane;   // 将采样的深度值重新转换回原始深度值 [0,1]
    //             shadow += currentDepth - bias1 > pcfDepth ? 1.0 : 0.0;        
    //         }
    //     } 
    // }
    // shadow /= (samples * samples * samples);


    // // PCF VER 2.0A     (设置一个固定的采样半径，基于当前片段与光源的距离动态调整采样数量，开销较小)
    // // ———————————————————————————————————————————————————————————————————————————————————-
    // float shadow  = 0.0;
    // float bias1   = max(0.05 * (1.0 - dot(fs_in.Normal, templightDir)), 0.15);
    // int samples = 20;
    // float viewDistance = length(viewPos - fs_in.FragPos);
    // float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0; // 根据视距动态调整采样半径
    // for(int i = 0; i < samples; ++i)
    // {
    //     float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
    //     closestDepth *= far_plane;   // undo mapping [0;1]
    //     if(currentDepth - bias > closestDepth)
    //         shadow += 1.0;
    // }
    // shadow /= float(samples); 
    

    // // PCF VER 2.0B     (设置采样数组，从中取固定数量的偏移量样本)
    // // -------------------------------------------------------
    // // 20个预定义的采样偏移方向，分布在一个单位球面上
    // vec3 sampleOffsetDirections[20] = vec3[]
    // (
    //     vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
    //     vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    //     vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    //     vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
    //     vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    // );  

    // float shadow  = 0.0;
    // float bias1   = max(0.05 * (1.0 - dot(fs_in.Normal, templightDir)), 0.15);
    // int samples = 20;
    // float viewDistance = length(viewPos - fs_in.FragPos);
    // float diskRadius = 0.05; 
    //     for(int i = 0; i < samples; ++i)
    // {
    //     float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
    //     closestDepth *= far_plane;   // undo mapping [0;1]
    //     if(currentDepth - bias > closestDepth)
    //         shadow += 1.0;
    // }
    // shadow /= float(samples); 


    // PCF VER 2.0B     (设置采样数组，从中取固定数量的偏移量样本)
    // -------------------------------------------------------
    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );  

    float shadow  = 0.0;
    float bias1   = max(0.05 * (1.0 - dot(fs_in.Normal, templightDir)), 0.15);
    int samples = 20;
    float viewDistance = length(viewPos - fs_in.FragPos);
    float diskRadius =  (1.0 + (viewDistance / far_plane)) / 25.0; // 根据视距动态调整采样半径
        for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias1 > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples); 



    return shadow;
}

// PCSS
// --- PCSS 阴影 --- 
float ShadowCalculationPCSS(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float receiverDepth = length(fragToLight);
    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 sampleDirs[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    // === Stage 1: Blocker Search ===
    float blockerSum = 0.0;
    int blockerCount = 0;
    float searchAngle = 0.05;

    for (int i = 0; i < 16; ++i)
    {
        vec3 probeDir = normalize(fragToLight + sampleDirs[i] * searchAngle);
        float probeDepth = texture(depthMap, probeDir).r * far_plane;

        float cosAngle = dot(normalize(fragToLight), probeDir);
        float zOffset = receiverDepth * (1.0 / max(cosAngle, 0.85) - 1.0);

        if (probeDepth < receiverDepth - zOffset - 0.02)
        {
            blockerSum += probeDepth;
            blockerCount++;
        }
    }

    // === Fallback: original PCF if not enough blockers ===
    if (blockerCount < 4)
    {
        float shadow = 0.0;
        float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.15);
        float viewDistance = length(viewPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

        for (int i = 0; i < 20; ++i)
        {
            float d = texture(depthMap, fragToLight + sampleDirs[i] * diskRadius).r * far_plane;
            if (receiverDepth - bias > d) shadow += 1.0;
        }
        shadow /= 20.0;
        return shadow;
    }

    float avgBlockerDepth = blockerSum / float(blockerCount);

    // === Stage 2: Penumbra ===
    // lightRadius: effective radius of the light source in world units
    float lightRadius = 0.5;
    float blockerDist = min(abs(avgBlockerDepth - receiverDepth) * 0.9, 10.0);
    float penumbraAngle = atan(lightRadius * blockerDist / (avgBlockerDepth * receiverDepth));
    penumbraAngle = clamp(penumbraAngle, 0.002, 0.2);

    // === Stage 3: PCF adaptive ===
    float shadow = 0.0;
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.15);

    for (int i = 0; i < 20; ++i)
    {
        float d = texture(depthMap, fragToLight + sampleDirs[i] * penumbraAngle).r * far_plane;
        if (receiverDepth - bias > d) shadow += 1.0;
    }
    shadow /= 20.0;

    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3) * lightIntensity;
    // 环境光
    vec3 ambient = 0.3 * lightColor;
    // 漫反射
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // 高光反射
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // 计算阴影
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}