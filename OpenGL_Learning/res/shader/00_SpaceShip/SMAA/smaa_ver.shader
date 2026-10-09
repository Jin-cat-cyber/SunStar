#version 330 core
//  SMAA 的全屏顶点着色器：与 bright_pass_ver / taa_ver 同构（属性布局一样：
//  location 0 = vec2 位置、location 1 = vec2 纹理坐标），因为都吃同一个 quadVAO。
//  抄一份到本目录是为了让 SMAA 这一组自包含，不依赖别的目录里的文件。
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}