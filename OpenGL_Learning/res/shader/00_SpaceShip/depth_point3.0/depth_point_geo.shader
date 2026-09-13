#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadowMatrices[6];
uniform int  faceIndex;      // -1 = 全部 6 面；0..5 = 只写该面

out vec4 FragPos;	// 片段位置（世界空间）

void main()
{
	int f0 = (faceIndex < 0) ? 0 : faceIndex;
	int f1 = (faceIndex < 0) ? 5 : faceIndex;

	for(int face = f0; face <= f1; ++face)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i)
		{
			FragPos = gl_in[i].gl_Position;
			gl_Position = shadowMatrices[face] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}