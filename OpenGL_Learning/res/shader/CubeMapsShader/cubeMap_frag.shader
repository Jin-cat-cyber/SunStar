// VER 1.0
// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;

// uniform sampler2D texture1;

// void main()
// {    
// 	FragColor = texture(texture1, TexCoords);
// }

// VER 2.0  反射光
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
	float ratio = 1.0 / 1.52; // 折射率
	vec3 I = normalize(Position - cameraPos);
	vec3 R = refract(I, normalize(Normal), ratio); // 折射光
	FragColor = vec4(texture(skybox, R).rgb, 1.0);
}