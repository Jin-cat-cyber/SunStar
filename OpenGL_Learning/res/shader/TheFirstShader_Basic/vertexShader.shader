// VER 1.0
// #version 330 core
// layout(location = 0) in vec3 aPos; // 顶点位置属性
// layout(location = 1) in vec3 aColor; // 顶点颜色属性

// out vec3 ourColor; // 输出颜色

// void main()
// {
//     gl_Position = vec4(aPos, 1.0); // 设置顶点位置
//     ourColor = aColor; // 传递顶点颜色
// }

//VER 2.0
// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec2 aTexCoord;

// out vec3 ourColor;
// out vec2 TexCoord;

// uniform mat4 transform;

// void main()
// {
//     gl_Position = transform * vec4(aPos, 1.0);
//     ourColor = aColor;
//     TexCoord = aTexCoord;
// }

// VER 3.0 -- ver5 camera
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}