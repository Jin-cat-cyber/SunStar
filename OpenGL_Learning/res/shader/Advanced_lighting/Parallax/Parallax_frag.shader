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
uniform sampler2D normalMap;
uniform	sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{
	// 从高度图中获取高度值，并计算视差偏移
	vec3 viewDir   = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoords = fs_in.TexCoords;

	texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
	// 如果新的纹理坐标超出范围，则丢弃该片段
	if(texCoords.x > 1.0 || texCoords.x < 0.0 || texCoords.y > 1.0 || texCoords.y < 0.0)
		discard;

	// 法线贴图获取法线向量
	vec3 normal = texture(normalMap, texCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0); // 这个法线在切线空间中
	
	// 光照计算
	vec3 color = texture(diffuseMap, texCoords).rgb;
	// 环境光
	vec3 ambient = 0.1 * color;
	// 漫反射
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	// 镜面反射
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

	vec3 specular = vec3(0.2) * spec;
	FragColor = vec4(ambient + diffuse + specular, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	// 从高度图中获取高度值
	float height = texture(depthMap, texCoords).r; // 这里假设高度图存储在红色通道中
	// 计算视差偏移
	vec2 p = viewDir.xy / viewDir.z * (height * heightScale); // 视差偏移量
	// 返回新的纹理坐标
	return texCoords - p;
}