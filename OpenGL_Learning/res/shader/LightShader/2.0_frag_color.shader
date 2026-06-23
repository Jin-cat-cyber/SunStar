// VER 6.0  点光源
// #version 330 core
// out vec4 FragColor;

// struct Material
// {
// 	sampler2D diffuse;
// 	sampler2D specular;
// 	float	  shininess;
// };

// struct Light
// {
// 	vec3 position;

// 	vec3 ambient;
// 	vec3 diffuse;
// 	vec3 specular;

// 	float constant;
//     float linear;
//     float quadratic;
// };

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 viewPos;
// uniform Material material;
// uniform Light light;

// void main()
// {
// 	// 环境光
// 	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

// 	// 漫反射
// 	vec3 norm = normalize(Normal);
// 	vec3 lightDir = normalize(light.position - FragPos);
// 	float diff = max(dot(norm, lightDir), 0.0); // 点积计算漫反射强度 背光为0，确保光强不为负数
// 	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

// 	// 镜面反射
// 	vec3 viewDir = normalize(viewPos - FragPos);
// 	vec3 reflectDir = reflect(-lightDir, norm);
// 	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
// 	// 正常光色
// 	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
// 	// 取反光色
// 	//vec3 specular = light.specular * spec * (vec3(1.0) - texture(material.specular, TexCoords).rgb); // 这里我们对采样到的高光色（specular color）进行反转处理，黑色会变为白色，白色会变为黑色。

// 	// 点光源衰减计算
// 	float distance    = length(light.position - FragPos);
// 	float attenuation = 1.0 / (light.constant + light.linear * distance + 
//     light.quadratic * (distance * distance));

// 	// 光照衰减
// 	ambient  *= attenuation; 
// 	diffuse  *= attenuation;
// 	specular *= attenuation;

// 	vec3 result = ambient + diffuse + specular;
// 	FragColor = vec4(result, 1.0);
// }


// VER 7.0  聚光灯
// #version 330 core
// out vec4 FragColor;

// struct Material
// {
// 	sampler2D diffuse;
// 	sampler2D specular;
// 	float	  shininess;
// };

// struct Light
// {
// 	vec3 position;
// 	vec3 direction;  // 聚光灯方向
// 	float cutOff;
// 	float outerCutOff;

// 	vec3 ambient;
// 	vec3 diffuse;
// 	vec3 specular;

// 	float constant;
//     float linear;
//     float quadratic;
// };

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 viewPos;
// uniform Material material;
// uniform Light light;

// void main()
// {
// 	vec3 lightDir = normalize(light.position - FragPos);

// 	float theta = dot(lightDir, normalize(-light.direction));
// 	float epsilon = light.cutOff - light.outerCutOff;
// 	float intensity = clamp(theta - Light.outerCutOff, 0.0, 1.0) / epsilon;)
	
// 	if(theta > light.cutOff)
// 	{

// 		float theta = dot(lightDir, normalize(-light.direction));
// 		// 环境光
// 		vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

// 		// 漫反射
// 		vec3 norm = normalize(Normal);
		
// 		float diff = max(dot(norm, lightDir), 0.0); // 点积计算漫反射强度 背光为0，确保光强不为负数
// 		vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

// 		// 镜面反射
// 		vec3 viewDir = normalize(viewPos - FragPos);
// 		vec3 reflectDir = reflect(-lightDir, norm);
// 		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
// 		// 正常光色
// 		vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
// 		// 取反光色
// 		//vec3 specular = light.specular * spec * (vec3(1.0) - texture(material.specular, TexCoords).rgb); // 这里我们对采样到的高光色（specular color）进行反转处理，黑色会变为白色，白色会变为黑色。

// 		// 点光源衰减计算
// 		float distance    = length(light.position - FragPos);
// 		float attenuation = 1.0 / (light.constant + light.linear * distance + 
// 		light.quadratic * (distance * distance));

// 		// 光照衰减
// 		//ambient  *= attenuation; 
// 		// 从环境光中移除衰减效果，否则在远距离处，由于 “else” 分支中的环境光项影响，聚光灯内部的光线会比外部更暗。
// 		diffuse  *= attenuation;
// 		specular *= attenuation;

// 		vec3 result = ambient + diffuse + specular;
// 		FragColor = vec4(result, 1.0);
// 	}
// 	else
// 	FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);
// }


// VER 7.1  聚光灯 软边缘
// #version 330 core
// out vec4 FragColor;

// struct	Material
// {
// 	sampler2D diffuse;
// 	sampler2D specular;
// 	float	  shininess;
// };

// struct Light
// {
// 	vec3 position;
// 	vec3 direction;
// 	float cutOff;
// 	float outerCutOff;

// 	vec3 ambient;
// 	vec3 diffuse;
// 	vec3 specular;

// 	float constant;
// 	float linear;
// 	float quadratic;
// };

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;

// uniform vec3 viewPos;
// uniform Material material;
// uniform Light light;

// void main()
// {
//     // 环境光
//     vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    
//     // 漫反射 
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(light.position - FragPos);
//     float diff = max(dot(norm, lightDir), 0.0);
//     vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
//     // 镜面反射
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, norm);  
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
    
//     // 聚光灯柔边缘
//     float theta = dot(lightDir, normalize(-light.direction)); 
//     float epsilon = (light.cutOff - light.outerCutOff);
//     float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//     diffuse  *= intensity;
//     specular *= intensity;
    
//     // 光照衰减
//     float distance    = length(light.position - FragPos);
//     float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
//     ambient  *= attenuation; 
//     diffuse   *= attenuation;
//     specular *= attenuation;   
        
//     vec3 result = ambient + diffuse + specular;
//     FragColor = vec4(result, 1.0);
// } 


// VER 8.0  混合光照
#version 330 core
out vec4 FragColor;

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

// 聚光灯
struct SpotLight
{
    vec3 position;
    vec3 direction;
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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 第一阶段：定向光照
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}


// 函数区域
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
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    return (ambient + diffuse + specular);
}

// 聚光灯
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}