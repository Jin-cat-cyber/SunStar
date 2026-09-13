#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBR;

in vec3 WorldPos;

uniform vec3 camPos;
uniform vec3 albedo;

void main()
{
	vec2 c = gl_PointCoord - 0.5;
	//float d = length(c);
	if (dot(c, c) > 0.25) discard;					// 方形点裁成圆
	
	//float a = smoothstep(0.5, 0.22, d);		// 柔和径向淡化
	vec3 N = normalize(camPos - WorldPos);	// 面向相机的法线(微粒近似)
	gPosition = WorldPos;
	gNormal   = N;
	gAlbedo	  = vec4(albedo, 0.0);		// 边缘渐隐 → 亚像素不再硬闪
	gPBR	  = vec4(0.9, 1.0, 0.0, 0.0);	// rough=0.9, ao=1

}
