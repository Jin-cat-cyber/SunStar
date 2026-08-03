// VER 1.0
// #version 330 core
// out vec4 FragColor;
  
// uniform vec3 objectColor;
// uniform vec3 lightColor;

// void main()
// {
//     FragColor = vec4(lightColor * objectColor, 1.0);
// }

// VER 2.0  		世界空间中计算冯氏光照模型
// #version 330 core
// out vec4 FragColor;

// in vec3 Normal;
// in vec3 FragPos;
  
// uniform vec3 objectColor;
// uniform vec3 lightColor;
// uniform vec3 lightPos;
// uniform vec3 viewPos;  // 观察位置


// void main()
// {
//     // Phong (冯氏) 光照模型的环境光部分
//     // 环境光创建过程解释
//     // 用光的颜色乘以一个很小的常量环境因子，再乘以物体的颜色，然后将最终结果作为片段的颜色

//     // Phong (冯氏) 光照模型的环境光部分
//     float ambientStrength = 0.1;    // 环境光强度
//     vec3 ambient = ambientStrength * lightColor;

//     vec3 norm = normalize(Normal);  // 法线向量归一化
//     vec3 lightDir = normalize(lightPos - FragPos);  // 光源方向向量归一化（从片段指向光源）

//     // Phong (冯氏) 光照模型的漫反射部分
//     float diff = max(dot(norm, lightDir), 0.0);     // 点积计算漫反射强度 背光为0，确保光强不为负数
//     vec3 diffuse = diff * lightColor;

//     // Phong (冯氏) 光照模型的镜面反射部分
//     float specularStrength = 0.5;   // 镜面反射强度
//     vec3 viewDir = normalize(viewPos - FragPos);    // 视线方向向量归一化（从片段指向观察者）
//     vec3 reflectDir = reflect(-lightDir, norm);

//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32是高光的散布因子(反光强度)
//     vec3 specular = specularStrength * spec * lightColor;

//     // 最终光源合成结果
//     vec3 result = (ambient + diffuse + specular) * objectColor;
//     FragColor = vec4(result, 1.0);
// }


// VER 2.1 		观察空间中计算冯氏光照模型(带有平行光)
// #version 330 core
// out vec4 FragColor;

// in vec3 FragPos;
// in vec3 Normal;
// in vec3 LightPos;  // 额外的变量，因为我们需要视图空间中的灯光位置，所以我们在顶点着色器中进行计算。

// uniform vec3 lightColor;
// uniform vec3 objectColor;

// void main()
// {
//     // 环境光
//     float ambientStrength = 0.1;
//     vec3 ambient = ambientStrength * lightColor;

//     // 漫反射
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(LightPos - FragPos);
//     float diff = max(dot(norm, lightDir), 0.0);
//     vec3 diffuse = diff * lightColor;

//     // 镜面反射
//     float specularStrength = 0.5;
//     vec3 viewDir = normalize(-FragPos); // 在视图空间中，观察者总是位于 (0,0,0)，所以视图方向 viewDir 就是 (0,0,0) - Position，即 -Position。
//     vec3 reflectDir = reflect(-lightDir, norm);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//     vec3 specular = specularStrength * spec * lightColor;

//     // 最终颜色
//     vec3 result = (ambient + diffuse + specular) * objectColor;
//     FragColor = vec4(result, 1.0);
// }

// VER 3.0  材质
// #version 330 core
// out vec4 FragColor;

// struct Material
// {
// 	vec3 ambient;
// 	vec3 diffuse;
// 	vec3 specular;
// 	float shininess;
// };

// struct Light
// {
// 	vec3 position;

// 	vec3 ambient;
// 	vec3 diffuse;
// 	vec3 specular;
// };

// in vec3 FragPos;
// in vec3 Normal;

// uniform vec3 viewPos;
// uniform Material material;
// uniform Light light;

// void main()
// {
// 	// 环境光
// 	vec3 ambient = light.ambient * material.ambient;

// 	// 漫反射
// 	vec3 norm = normalize(Normal);
// 	vec3 lightDir = normalize(light.position - FragPos);
// 	float diff = max(dot(norm, lightDir), 0.0); // 点积计算漫反射强度 背光为0，确保光强不为负数
// 	vec3 diffuse = light.diffuse * (diff * material.diffuse);

// 	// 镜面反射
// 	vec3 viewDir = normalize(viewPos - FragPos);
// 	vec3 reflectDir = reflect(-lightDir, norm);
// 	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
// 	vec3 specular = light.specular * (spec * material.specular);

// 	vec3 result = ambient + diffuse + specular;
// 	FragColor = vec4(result, 1.0);
// }


// VER 4.0  漫反射贴图
#version 330 core
out vec4 FragColor;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float	  shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
	// 环境光
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

	// 漫反射
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0); // 点积计算漫反射强度 背光为0，确保光强不为负数
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

	// 镜面反射
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}