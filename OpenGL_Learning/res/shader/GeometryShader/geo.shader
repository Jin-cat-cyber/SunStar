// #version 330 core
// layout (points) in;
// layout (triangle_strip, max_vertices = 5) out;

// in VS_OUT
// {
// 	vec3 color;
// } gs_in[];

// out vec3 fColor;

// void build_house(vec4 position)
// {
// 	fColor = gs_in[0].color;
// 	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
// 	EmitVertex();
// 	gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:bottom-right
//     EmitVertex();
//     gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // 3:top-left
//     EmitVertex();
//     gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // 4:top-right
//     EmitVertex();
//     gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
//     fColor = vec3(1.0, 1.0, 1.0);
//     EmitVertex();
//     EndPrimitive();
// }

// void main()
// {
//     build_house(gl_in[0].gl_Position);
// }


// // VER 2.0  Boom Model
// #version 330 core
// layout (triangles) in;
// layout (triangle_strip, max_vertices = 3) out;

// in VS_OUT {
//     vec2 texCoords;
// } gs_in[];

// out vec2 TexCoords; 

// uniform float time;

// vec4 explode(vec4 position, vec3 normal)
// {
//     float magnitude = 2.0;
//     vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;    // 方向与幅度，通过时间变化控制爆炸效果
//     return position + vec4(direction, 0.0);
// }

// vec3 GetNormal()
// {
//     vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);   // 获得两个顶点的向量
//     vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);   // 同上
//     return normalize(cross(a, b));  // 通过叉乘获得法线并归一化
// }

// void main() {    
//     vec3 normal = GetNormal();

//     gl_Position = explode(gl_in[0].gl_Position, normal);
//     TexCoords = gs_in[0].texCoords;
//     EmitVertex();
//     gl_Position = explode(gl_in[1].gl_Position, normal);
//     TexCoords = gs_in[1].texCoords;
//     EmitVertex();
//     gl_Position = explode(gl_in[2].gl_Position, normal);
//     TexCoords = gs_in[2].texCoords;
//     EmitVertex();
//     EndPrimitive();
// }


// VER 3.0  Normal Model
#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.2;	//  生成法线的长度

uniform mat4 projection;

void GenerateLine(int index)
{
	gl_Position = projection * gl_in[index].gl_Position;	// 起点位置，即该顶点在视觉世界中的位置
	EmitVertex();
	gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);		// 终点位置，从起点到终点长度为MAGNITUDE的法线
	EmitVertex();
	EndPrimitive();
}

void main()
{
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
}