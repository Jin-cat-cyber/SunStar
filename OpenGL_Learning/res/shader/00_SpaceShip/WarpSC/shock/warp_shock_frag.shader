#version 330 core
// 冲击波的片元着色器（从 warp_debris_frag 那份 fork 出来的独立副本，改它不影响碎屑/光柱）。
// 与碎屑那份的关键差别：碎屑是实心块，用 rim 菲涅尔"掠射亮、正对暗"；
// 而【环是平的】，法线处处平行于纵轴 —— 用 rim 会让它正对时全黑、侧对时最亮，正好反了。
// 这里改成"正对相机才亮"，并且按半径把环带以外的部分丢掉。

out vec4 FragColor;

in vec3  vWorldPos;
in vec3  vNormal;
in vec3  vObjPos;
in vec3  vTint;
in float vAlpha;

uniform vec3  camPos;
uniform vec3  uDebrisColor;      // 沿用这个名字（这一份是波自己的）
uniform float uDebrisStrength;   // = U_SHOCK_GAIN × 能量律 × 窗口包络
uniform float uRingR;            // 当前环半径（世界单位）
//  【现行】网格的世界外半径 = 环半径 + 2.5 倍带宽。系数不能小于 sqrt(ln(1/0.002)) = 2.4929，
//  否则下面那个 discard 还没生效网格就先到边，圆盘外缘会留下一条硬台阶。
//  以下那行的注释是旧值（2 倍），留档对比：
uniform float uRingScale;        // 网格的世界外半径 = 环半径 + 2×带宽（见 C++ 侧）
uniform float uBandW;            // 环带的世界半宽
uniform float uFacingFloor;      // 侧对时的可见度下限（0 = 纯的：正侧看即消失）

void main()
{
    //  世界半径 = 物体空间半径 × model 在 XZ 上的缩放（也就是当前环半径）。
    //  环带用片元切而不是用网格做，是为了让它的【世界宽度】恒定 ——
    //  网格若直接把带做出来，带会随半径一起放大。
    float objR   = length(vObjPos.xz);
    float worldR = objR * uRingScale;      // 注意用 uRingScale，不是 uRingR

    //  高斯环带：圆顶 + 长尾，比"平顶的 smoothstep"柔得多；
    //  而且和扭曲用的剖面（shockProfile 也是高斯）是同一个形状 —— 两处读起来才像一件事。
    //  高斯没有硬边界，长尾会让环看起来比原来宽：想保持原来的视觉厚度就把 S_BAND_W 调小。
    float xb   = (worldR - uRingR) / uBandW;
    float band = exp(-xb * xb);
    if (band <= 0.002) discard;

    vec3 V = normalize(camPos - vWorldPos);

    //  正对度：环面法线就是纵轴，所以 |dot(N, V)| 直接给出"环面正对相机的程度"。
    float facing = abs(dot(normalize(vNormal), V));
    //facing = mix(uFacingFloor, 1.0, facing);
    facing = mix(uFacingFloor, 1.0, pow(facing, 0.7));

    vec3 col = uDebrisColor * vTint * band * facing;

    FragColor = vec4(col * vAlpha * uDebrisStrength, 1.0);
}