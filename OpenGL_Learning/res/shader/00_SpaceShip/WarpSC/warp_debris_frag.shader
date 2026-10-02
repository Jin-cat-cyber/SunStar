#version 330 core
out vec4 FragColor;

in vec3  vWorldPos;
in vec3  vNormal;
in vec3  vTint;
in float vAlpha;

uniform vec3  camPos;
uniform vec3  uDebrisColor;      // 碎屑基色
uniform float uDebrisStrength;   // 整体强度
uniform float uDebrisEdge;       // 正对处的基础亮度，0 = 只有边缘亮

void main()
{
	vec3 V = normalize(camPos - vWorldPos);

    // 菲涅尔：掠射角亮、正对处暗。取绝对值是因为本工程从不启用面剔除，背面也会被画到 ——
    // 碎屑是实心块，正反两面都参与反而更像"块"而不是"片"。
    float rim = pow(1.0 - abs(dot(normalize(vNormal), V)), 2.0);

    // 加法混合（GL_ONE, GL_ONE），所以这里输出的是"要加多少"，不是最终颜色。
    vec3 col = uDebrisColor * vTint * (uDebrisEdge + rim);

    FragColor = vec4(col * vAlpha * uDebrisStrength, 1.0);
}
