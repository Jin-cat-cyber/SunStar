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
uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	// 步长数上下限
	const float minLayers = 8;
	const float maxLayers = 32;
	// 步长
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	// 计算每层的深度[0, 1]之间
	float layerDepth = 1.0 / numLayers;
	// 当前深度
	float currentLayerDepth = 0.0;
	// 计算视差总偏移
	vec2 P = viewDir.xy / viewDir.z * heightScale;
	// 每层的纹理坐标偏移
    vec2 deltaTexCoords = P / numLayers;

	// 初始化当前纹理坐标和深度图的值
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		// 沿视线方向前进一个步长
		currentTexCoords -= deltaTexCoords;
		// 更新当前深度图的值
		currentDepthMapValue = texture(depthMap, currentTexCoords).r;
		// 增加当前层的深度
		currentLayerDepth += layerDepth;
	}

	// 线性插值，获取更精确的纹理坐标
	// 获取碰撞前的纹理坐标（执行反向操作）
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// 获取碰撞前后的深度值，用于线性插值
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	// 纹理坐标插值
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

void main()
{
	// 从高度贴图中获取高度值，并计算视差偏移
	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoords = fs_in.TexCoords;

	texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
	// 如果新的纹理坐标超出范围，则丢弃该片段
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
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