#version 330 core
layout (location = 0) in vec3 aPos;


layout (location = 3) in mat4 aInstanceMatrix;

uniform mat4 model;
uniform bool instanced;

void main()
{
	mat4 r_model = instanced ? aInstanceMatrix : model;
	gl_Position = r_model * vec4(aPos, 1.0);
}