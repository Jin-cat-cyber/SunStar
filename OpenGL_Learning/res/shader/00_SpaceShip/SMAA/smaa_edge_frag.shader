#version 330 core
//  SMAA 第一遍：边缘检测。输出两个标记，各占一个通道：
//    .r = 本像素与【左】邻居之间有一条亮度突变（一条竖边）
//    .g = 本像素与【下】邻居之间有一条亮度突变（一条横边）
//  只记"左/下"一侧是刻意的：同一条边在另一侧的像素那里会被记成它的"另一条边"，
//  于是"连续的同侧标记"恰好就是同一条边的一段，第二遍沿着它走就能找到端点。
in vec2 TexCoords;

uniform sampler2D uColor;
uniform vec2  uTexel;          // 1.0 / 渲染尺寸（像素）
uniform float uThreshold;      // 绝对亮度差阈值（起点 0.06）
uniform float uLocalContrast;  // 局部对比自适应系数（起点 0.15；0 = 关闭）

out vec4 FragColor;

float Luma(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    float c = Luma(texture(uColor, TexCoords).rgb);
    float l = Luma(texture(uColor, TexCoords - vec2(uTexel.x, 0.0)).rgb);
    float d = Luma(texture(uColor, TexCoords - vec2(0.0, uTexel.y)).rgb);

    //  阈值随两侧中较亮的那一个抬高：亮的地方（高光、太阳附近）对比天然更大，
    //  不抬高就会把那里的纹理细节当成边来磨 —— 这正是 SMAA 与 FXAA 的分界线之一。
    float thrL = max(uThreshold, uLocalContrast * max(c, l));
    float thrD = max(uThreshold, uLocalContrast * max(c, d));

    float eL = (abs(c - l) > thrL) ? 1.0 : 0.0;
    float eD = (abs(c - d) > thrD) ? 1.0 : 0.0;

    FragColor = vec4(eL, eD, 0.0, 1.0);
}