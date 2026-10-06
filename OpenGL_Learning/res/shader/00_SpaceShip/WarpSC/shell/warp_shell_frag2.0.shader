#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vObjPos;

uniform vec3  camPos;
uniform vec3  uShellColor;		// 电流色
uniform float uShellStrength;	// 由 shellAlpha(tau) 驱动
uniform float uObjRadius;		// 模型局部坐标的包围半径，用来把 vObjPos 归一化
uniform float uTime;

// SC 分支新增：多边形能量网格
uniform float uGridFreq;		// 每单位归一化坐标的格数（越大格越密）
uniform float uGridWidth;		// 格线宽度（以单元格为单位，0.02 ~ 0.06 之间取）
uniform float uGridMix;			// 第二组 45° 格线的权重：0 = 只有正方格，1 = 两组叠加
uniform float uGridStrength;	// 网格整体强度

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

// 一组三向正交格线：返回 0..1 的线强度，格面上为 1、格心为 0。
// q * n 取 fract 之后，0 与 1 就是格面；这里用 |fract - 0.5|（格心 0、格面 0.5），
// 越靠近 0.5 越亮。三个轴各自成族，取最大值就是三族的并集 ——
// 于是格线是三维的、贴在舰体上，不是屏幕上的方格。
// 用 fract 而不是 fmod：GLSL 的 fract(x) = x - floor(x)，负坐标同样落在 [0,1)。
float lattice(vec3 q, float n, float w)
{
	vec3 c = abs(fract(q * n) - 0.5);
	vec3 d = smoothstep(0.5 - w, 0.5, c);
	return max(max(d.x, d.y), d.z);
}

void main()
{
    vec3 V = normalize(camPos - vWorldPos);

    // 菲涅尔：掠射角亮、正对处暗。这一项让外壳读起来是"裹住"，而不是"镀了一层"。
    float fres = pow(1.0 - clamp(dot(vNormal, V), 0.0, 1.0), 6.0);

    // 细丝只在掠射区附近可见，否则正对镜头那一面会糊成一片亮斑。
    vec3  q    = vObjPos / max(uObjRadius, 1e-3);
    float wire = filaments(q, uTime) * smoothstep(0.15, 0.85, fres);

    // ===== SC 分支：多边形能量网格 =====
    // 第一组：三向正交格（正方格）。
    float g1 = lattice(q, uGridFreq, uGridWidth);
    // 第二组：把坐标先绕 Z 轴转 45 度（(x+y)/√2, (y-x)/√2, z）再取格，
    // 它的格面斜切第一组的方格，叠起来就读作三角/菱形 —— 这就是"多边形"的来源。
    vec3  q2 = vec3((q.x + q.y) * 0.70710678, (q.y - q.x) * 0.70710678, q.z);
    float g2 = lattice(q2, uGridFreq, uGridWidth);
    // uGridMix = 0 时只有第一组（等价方案 A），= 1 时两组叠加（方案 B）。
    float grid = mix(g1, max(g1, g2), uGridMix);
    // 正对镜头的那部分压暗一些，否则整块面会一起变亮、丢掉"网格浮在壳上"的层次。
    grid *= smoothstep(0.05, 0.55, fres);

    // 加法混合（GL_ONE, GL_ONE），所以这里输出的是"要加多少"，不是最终颜色。
    // 系数取到略大于 1，峰值时才会被 bright pass 拾取、进 bloom 晕开。
    vec3 col = uShellColor * (fres * 1.6 + wire * 0.9 + grid * uGridStrength);
    FragColor = vec4(col * uShellStrength, 1.0);
}