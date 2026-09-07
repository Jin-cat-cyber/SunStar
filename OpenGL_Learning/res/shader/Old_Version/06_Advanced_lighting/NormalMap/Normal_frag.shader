#version 330 core
out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
} fs_in;


uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	// 从UV贴图中取出将法线映射到[0, 1]范围内
	vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
	// 将法线映射到[-1, 1]范围内
	normal = normalize(normal * 2.0 - 1.0); // this normal is in tangent space

	// 获得漫反射贴图的颜色
	vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
	// 环境光
	vec3 ambient = 0.1 * color;
	// 漫反射
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	// vec3 lightDir = normalize(fs_in.TangentLightPos - normalize(fs_in.TangentFragPos));	// 这种写法会导致光照方向不正确，无法正确计算漫反射和镜面反射
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	// 镜面反射
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.2) * spec; // 白色高光
	vec3 result = ambient + diffuse + specular;
	//result = pow(result, vec3(1.0/2.2)); // Gamma校正
	FragColor = vec4(result, 1.0);

	// FragColor = vec4(normal * 0.5 + 0.5, 1.0);
	// FragColor = vec4(lightDir * 0.5 + 0.5, 1.0);
}