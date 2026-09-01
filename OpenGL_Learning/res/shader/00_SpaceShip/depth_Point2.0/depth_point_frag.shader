#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

out vec4 FragColor;

void main()
{
	float lightDistance = length(FragPos.xyz - lightPos);
	float z = lightDistance / far_plane;

	// 通过将距离除以far_plane，将距离值归一化到[0,1]范围内
	//lightDistance = lightDistance / far_plane;

	FragColor = vec4(z, z * z, 0.0, 0.0);
	// 将距离值存储在深度缓冲区中
	gl_FragDepth = z;
}
