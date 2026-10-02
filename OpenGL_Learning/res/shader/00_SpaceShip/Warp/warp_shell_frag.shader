#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vObjPos;

uniform vec3  camPos;
uniform vec3  uShellColor;		// 电流色
uniform float uShellStrength;	// 由电流通道 u_c 驱动
uniform float uObjRadius;		// 模型局部坐标的包围半径，用来把 vObjPos 归一化
uniform float uTime;

// 三组不同频率与走向的 sin 相乘再取绝对值：乘积接近零的地方形成细亮的脊线，
// 读起来就是一束束沿舰体爬的电流。比完整的 3D 值噪声便宜，而"细丝"本来
// 就是这种叠层条纹的样子。
// 入参 q 是【归一化后的物体空间坐标】（量级约 ±1），所以频率能直接用 O(10) 的
// 常数 —— 这正是要除以 uObjRadius 的原因：否则同一组常数会因为模型尺度
// （这里大 2000 倍）变成一片噪点。
float filaments(vec3 q, float t)
{
	float a = sin(q.y * 9.0 + t * 3.0);
	float b = sin((q.x + q.z) * 6.0 - t * 2.2);
	float c = sin(dot(q, vec3(4.0, -7.0, 5.0)) + t * 4.1);
	return 1.0 - pow(abs(a * b * c), 0.35);
}

void main()
{
    vec3 V = normalize(camPos - vWorldPos);

    // 菲涅尔：掠射角亮、正对处暗。这一项让外壳读起来是"裹住"，而不是"镀了一层"。
    float fres = pow(1.0 - clamp(dot(vNormal, V), 0.0, 1.0), 3.0);

    // 细丝只在掠射区附近可见，否则正对镜头那一面会糊成一片亮斑。
    vec3  q    = vObjPos / max(uObjRadius, 1e-3);
    float wire = filaments(q, uTime) * smoothstep(0.15, 0.85, fres);

    // 加法混合（GL_ONE, GL_ONE），所以这里输出的是"要加多少"，不是最终颜色。
    // 系数取到略大于 1，峰值时才会被 bright pass 拾取、进 bloom 晕开。
    vec3 col = uShellColor * (fres * 1.6 + wire * 0.9);
    FragColor = vec4(col * uShellStrength, 1.0);
}