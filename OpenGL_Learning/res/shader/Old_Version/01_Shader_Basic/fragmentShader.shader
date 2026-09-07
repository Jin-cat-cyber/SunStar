// VER 1.0
// #version 330 core
// out vec4 FragColor; // 输出颜色
// in vec3 ourColor; // 接收来自顶点着色器的颜色

// void main()
// {
//     FragColor = vec4(ourColor, 1.0); // 设置片段颜色
// }

// VER 2.0
// #version 330 core
// out vec4 FragColor;
  
// in vec3 ourColor;
// in vec2 TexCoord;

// uniform sampler2D texture1;
// uniform sampler2D texture2;

// void main()
// {
//     FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
//     // 最终输出颜色现在是两个纹理的结合。
//     // GLSL内建的mix函数需要接受两个值作为参数，并对它们根据第三个参数进行线性插值。
//     // 如果第三个值是0.0，它会返回第一个输入；如果是1.0，会返回第二个输入值。
//     // 0.2会返回80%的第一个输入颜色和20%的第二个输入颜色，即返回两个纹理的混合色。
// }

// VER 3.0 -- ver5 camera
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// 纹理采样器
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// 线性插值两个纹理（80% container, 20% awesomeface）
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	// 最终输出颜色现在是两个纹理的结合。
}