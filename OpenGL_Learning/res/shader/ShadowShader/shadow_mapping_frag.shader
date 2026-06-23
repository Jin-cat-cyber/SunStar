#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCaculation(vec4 fragPosLightSpace)
{
    vec3 templightDir = normalize(lightPos - fs_in.FragPos);
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 将坐标转换到 [0, 1] 范围
    projCoords = projCoords * 0.5 + 0.5;
    // 获取当前片段在深度贴图中的深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // 获取当前片段的深度值
    float currentDepth = projCoords.z;
    // 进行阴影测试
    float bias1 = 0.005; // (常数偏移，阴影偏移，防止阴影失真,可以根据场景调整
    float bias2 = max(0.05 * (1.0 - dot(fs_in.Normal, templightDir)), 0.005); // (基于法线和光线方向的动态偏移，增加阴影的稳定性)(偏移值高度依赖于光源与表面的夹角)
    //  float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    // float shadow = currentDepth - bias2 > closestDepth ? 1.0 : 0.0;

    // PCF (Percentage Closer Filtering) 进行阴影平滑
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias2 > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    // 如果片段在光源的远平面之外，则不进行阴影计算
    // 这可以防止在远平面之外的片段被错误地认为在阴影中
    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }

    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    // 环境光
    vec3 ambient = 0.15 * lightColor;
    // 漫反射
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // 镜面反射
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // 计算阴影
    float shadow = ShadowCaculation(fs_in.FragPosLightSpace);
    // 将结果应用到片段颜色上
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);

}