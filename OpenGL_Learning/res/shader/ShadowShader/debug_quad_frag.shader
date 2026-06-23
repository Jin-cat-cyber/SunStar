#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

// 使用透视投影矩阵时必须的
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // 将深度值从 [0, 1] 转换到 [-1, 1]
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() 
{             
	// 从深度贴图中获取深度值
	float depthValue = texture(depthMap, TexCoords).r;
	// 将深度值线性化
	//float linearDepth = LinearizeDepth(depthValue) / far_plane; // 透视光
	// 归一化到 [0, 1]

	FragColor = vec4(vec3(depthValue), 1.0); // 平行光 
	// 输出灰度颜色 ， 如果需要线性深度，可以使用 linearDepth 替换 depthValue
}