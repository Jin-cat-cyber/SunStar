#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
	vec3 TangentDirLightDir;	// 切空间定向光方向
	vec3 TangentSpotLightDir;	// 切空间聚光灯方向
} fs_in;

// 定向光
struct DirLight
{
    // vec3 direction;

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

// 聚光灯
struct SpotLight
{
    vec3 position;
    // vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};
#define NR_POINT_LIGHTS 4    // 点光源数量

// 是否开启Gamma校正
uniform bool gamma;

// 采样器
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 color, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 color, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 color, vec3 fragPos, vec3 viewDir);


void main()
{
	// 从UV贴图中取出将法线映射到[0, 1]范围内
	vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
	// 将法线映射到[-1, 1]范围内
	normal = normalize(normal * 2.0 - 1.0); // this normal is in tangent space

	// 获得漫反射贴图的颜色
	vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    // 获得光照方向
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    // 获得视线方向
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    // 第一阶段：定向光照
    vec3 result = CalcDirLight(dirLight, normal, color, viewDir);
    // 第二阶段：点光源
    //for(int i = 0; i < NR_POINT_LIGHTS; i++)
    // result = vec3(0.0);
    result += CalcPointLight(pointLights[0], normal, color, fs_in.FragPos, viewDir);    
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, normal, color, fs_in.FragPos, viewDir);    
    
    if (gamma)
    {
        result = pow(result, vec3(1.0/2.2)); // Gamma校正
    }
    FragColor = vec4(result, 1.0); 
    //FragColor = vec4(fs_in.TangentSpotLightDir * 0.5 + 0.5, 1.0);
	
}


// 平行光(定向光)
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 color, vec3 viewDir)
{
	// 获得漫反射贴图的颜色
    //vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

	// 漫反射
    vec3 lightDir = normalize(fs_in.TangentDirLightDir);
	//vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);

	// 镜面反射
	vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);;

    vec3 ambient = light.ambient * color;           // * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * color;    // * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * spec;          // * texture(material.specular, TexCoords).rgb;
    return (ambient + diffuse + specular);
}


// 点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 color, vec3 fragPos, vec3 viewDir)
{
	// 获得漫反射贴图的颜色
    //vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
	// 漫反射
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);

	// 镜面反射
	vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	// 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * color;            // * texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * color;     // * texture(material.diffuse, TexCoords).rgb;
    vec3 specular = light.specular * spec;           // * texture(material.specular, TexCoords).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


// 聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 color, vec3 fragPos, vec3 viewDir)
{
	// 获得漫反射贴图的颜色
    //vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

	// 漫反射
    vec3 lightDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	// 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

	// 聚光强度
    float theta = dot(lightDir, normalize(fs_in.TangentSpotLightDir)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * color;                // * vec3(texture(floorTexture, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * color;         // * vec3(texture(floorTexture, fs_in.TexCoords));
    vec3 specular = light.specular * spec;               // * vec3(texture(material.specular, TexCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular) * 0.6;        // * 0.6是为了让聚光灯的亮度不至于过高
}