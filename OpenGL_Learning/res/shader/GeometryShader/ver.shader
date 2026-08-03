// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;

// out VS_OUT 
// {
// 	vec3 color;
// } vs_out;

// void main()
// {
// 	vs_out.color = aColor;
// 	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
// }


// // VER 2.0  Boom Model
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 2) in vec2 aTexCoords;

// out VS_OUT {
//     vec2 texCoords;
// } vs_out;

// uniform mat4 projection;
// uniform mat4 view;
// uniform mat4 model;

// void main()
// {
//     vs_out.texCoords = aTexCoords;
//     gl_Position = projection * view * model * vec4(aPos, 1.0); 
// }


// VER 3.0  Normal Model
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT
{
	vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	vs_out.normal = vec3(vec4(normalMatrix * aNormal, 0.0));
	gl_Position = view * model * vec4(aPos, 1.0);
}