#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

layout (location = 0) out vec2 vsmMoments;

void main()
{
	float linearDepth = length(FragPos.xyz - lightPos) / far_plane;
	gl_FragDepth = linearDepth;

	// float dx = dFdx(linearDepth);
	// float dy = dFdy(linearDepth);
	float varianceFloor = 0.001;
	vsmMoments.x = linearDepth;
	//vsmMoments.y = linearDepth * linearDepth + 0.25 * (dx * dx + dy * dy);
	vsmMoments.y = linearDepth * linearDepth + varianceFloor;
}