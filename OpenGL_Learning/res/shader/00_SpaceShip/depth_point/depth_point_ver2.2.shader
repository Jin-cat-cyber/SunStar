#version 330 core
layout (location = 0) in vec3 aPos;


layout (location = 3) in mat4 aInstanceMatrix;

uniform mat4 model;
uniform bool instanced;

void main()
{
	if(instanced)
		gl_Position = aInstanceMatrix * vec4(aPos, 1.0);
	else
		gl_Position = model * vec4(aPos, 1.0);
}