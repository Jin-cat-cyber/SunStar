#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

//out vec2 TexCoords;

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool reverse_normals;

void main()
{
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	if(reverse_normals)
		vs_out.Normal = mat3(transpose(inverse(model))) * -aNormal;	
		//	这是因为在某些情况下，模型的法线可能需要反转，
		//	例如当模型被镜像时。通过将法线乘以-1，我们可以确保它们正确地指向外部，从而获得正确的光照效果。
	else
		vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
	vs_out.TexCoords = aTexCoords;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}