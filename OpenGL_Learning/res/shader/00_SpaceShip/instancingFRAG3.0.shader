// VER 3.0
#version 330 core
out vec4 FragColor;

// 材质
struct	Material
{
	sampler2D diffuse;
	sampler2D specular;
	float	  shininess;
};

// 定向光
struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// 点光源
struct PointLight
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4    // 点光源数量

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

// --- 阴影 ---
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform float far_plane;
uniform bool shadows;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);                   // 计算定向光
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow); // 计算点光源
float ShadowCalculation(vec3 fragPos);      // 计算阴影

void main()
{
    // 属性
    vec3 norm = normalize(Normal); // 确保归一化
    vec3 viewDir = normalize(viewPos - FragPos);

    float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;

    // 第一阶段：定向光照
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源（只用第0个）
    result += CalcPointLight(pointLights[0], norm, FragPos, viewDir, shadow);

    // 简单 Reinhard 色调映射（防止高亮饱和）与伽马校正
    // vec3 mapped = result / (result + vec3(1.0)); // Reinhard
    // mapped = pow(mapped, vec3(1.0 / 2.2)); // gamma correction


    //FragColor = vec4(mapped, 1.0);
    FragColor = vec4(result, 1.0);
}

// 函数区
// 定向光
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    return (ambient + diffuse + specular);
}

// 点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    //return (ambient + diffuse + specular);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

// 计算阴影
float ShadowCalculation(vec3 fragPos)
{
    vec3 templightDir = normalize(lightPos - FragPos);
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
    float bias1   = max(0.05 * (1.0 - dot(Normal, templightDir)), 7.5);
    int samples = 20;
    float viewDistance = length(viewPos - FragPos);
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
