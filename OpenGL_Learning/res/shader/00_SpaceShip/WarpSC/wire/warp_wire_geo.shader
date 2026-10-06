#version 330 core
// 给三角形的三个角分别打上重心坐标 (1,0,0) / (0,1,0) / (0,0,1)。
// 片元里"某个分量为 0"的位置就是一条边 —— 这样画出来的线就是三角形真正的边，
// 不多不少，而且线宽可以在片元里按屏幕空间控制。
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vObjPos[];

out vec3 gBary;
out vec3 gObjPos;

void main()
{
    gBary = vec3(1.0, 0.0, 0.0);
    gObjPos = vObjPos[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gBary = vec3(0.0, 1.0, 0.0);
    gObjPos = vObjPos[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    gBary = vec3(0.0, 0.0, 1.0);
    gObjPos = vObjPos[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}