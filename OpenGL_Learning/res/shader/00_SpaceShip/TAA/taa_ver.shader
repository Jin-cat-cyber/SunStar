#version 330 core
// 冲击波扭曲 pass 的全屏顶点着色器：与 bloom 那份 bright_pass_ver 同构
// （属性布局也一样：location 0 = vec2 位置、location 1 = vec2 纹理坐标），
// 因为两者都吃同一个 quadVAO。抄一份过来是为了让这一组着色器自包含。
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
