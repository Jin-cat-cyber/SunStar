#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{	   
	FragColor = texture(skybox, TexCoords);
}

// VER 2.0  动态天空盒  失败尝试
// #version 330 core
// out vec4 FragColor;

// in vec3 TexCoords;

// uniform samplerCube skybox;
// uniform float time;				// 时间统一变量
// uniform vec3 sunDirection;		// 太阳方向
// uniform float dayNightCycle;	// 昼夜循环参数，范围0.0（午夜）到1.0（正午）

// void main()
// {	
// 	vec3 originalColor = texture(skybox, TexCoords).rgb;
// 	FragColor = texture(skybox, TexCoords);
// }

// vec3 caculateDayNightColor(vec3 originalColor, vec3 direction)
// {
// 	// 日夜基础颜色
// 	vec3 dayColor = originalColor; // 白天颜色
// 	vec3 nightColor = originalColor * vec3(0.1, 0.1, 0.3); // 夜晚颜色，调暗并偏蓝
// 	baseColor = mix(nightColor, dayColor, dayNightCycle);)

// 	// 根据太阳位置计算光照强度
// 	float sunIntensity = max(dot(normalize(direction), normalize(sunDirection)), 0.0);
// 	float dawnFactor

// 	float dawnColor = mix(vec3(1.0, 0.5, 0.3), baseColor, dawnFactor);

// 	return mix(baseColor, dawnColor, dawnFactor * 0.5);
// }

// VER 3.0	动态天空盒  再次尝试     不符合预期，舍弃
// #version 330 core
// out vec4 FragColor;
// in vec3 TexCoords;

// uniform float time; // 传入的时间参数
// uniform vec3 sunDirection; // 太阳方向（单位向量）
// uniform float daNightCycle; // 日夜循环（0~1）

// void main()
// {
//     // 归一化方向
//     vec3 dir = normalize(TexCoords);

//     // 天空主色（可用大气散射公式替换）
//     vec3 dayColor = vec3(0.3, 0.6, 1.0);   // 白天蓝色
//     vec3 nightColor = vec3(0.02, 0.02, 0.08); // 夜晚深蓝

//     // 太阳高光
//     float sunAmount = max(dot(dir, normalize(sunDirection)), 0.0);
//     vec3 sunColor = vec3(1.0, 0.9, 0.6) * pow(sunAmount, 1000.0);

//     // 线性插值日夜
//     vec3 skyColor = mix(nightColor, dayColor, daNightCycle);

//     // 叠加太阳
//     skyColor += sunColor;

//     FragColor = vec4(skyColor, 1.0);
// }