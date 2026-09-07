#version 330 core
out vec4 FragColor;

uniform vec3  camPos;
uniform vec3  planetCenter;
uniform float planetRadius;    // 火星球半径（世界）
uniform float atmoScale;       // 大气壳半径倍数（如 1.2）
uniform vec3  sunDir;          // 太阳方向（归一化，从行星指向太阳）
uniform vec3  sunColor;        // 太阳颜色
uniform float density;         // 大气密度（瑞利）
uniform vec3  rayleighCoef;    // 瑞利散射系数（偏蓝）
uniform float miecoef;         // 米氏散射系数（雾）
uniform vec2  resolution;
uniform mat4  invProjView;
uniform float intensity;        // 大气总亮度旋钮（B+C 共用），建议初值 8~12
uniform float falloff = 5.0;    // 密度衰减系数（改小=壳变厚软，利于 C）

// --- 简洁版大气散射（瑞利 + 米氏 + Beer-Lambert）---
vec2 raySphere(vec3 ro, vec3 rd, vec3 c, float R)
{
	vec3 oc = ro - c;
	float b = dot(oc, rd);
	float c2 = dot(oc, oc) - R*R;
	float disc = b*b - c2;
	if(disc < 0.0) return vec2(1.0, 0.0);
	float sq = sqrt(disc);

	return vec2(-b - sq, -b + sq);
}

// 瑞利相位函数（散射角）
float  phaseRayleigh(float cosTheta)
{
	return 0.0596831 * (1.0 + cosTheta * cosTheta);
}

// 米氏相位函数（前向散射， cosTheta 接近1最亮）
float phaseMie(float cosTheta)
{
	float g = 0.72;
	float g2 = g * g;
	return 0.0795775 * ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

void main()
{
	// 1.重建视线
	vec2 ndc = gl_FragCoord.xy / resolution * 2.0 - 1.0;
	vec4 farW = invProjView * vec4(ndc, 1.0, 1.0);
	farW /= farW.w;
	vec3 rd = normalize(farW.xyz - camPos);

	// 2.大气壳进出点
	// 计算视线与大气球体的交点
	float atmoR = planetRadius * atmoScale;
	vec2 atmo = raySphere(camPos, rd, planetCenter, atmoR);
	float tAtmoEnter = max(atmo.x, 0.0);

    mat4 projView = inverse(invProjView);
    vec3 enterP = camPos + rd * tAtmoEnter;
    vec4 ec = projView * vec4(enterP, 1.0);
    gl_FragDepth = ec.z / ec.w * 0.5 + 0.5;   // 大气壳最近交点的窗口深度
	
    float tAtmoExit  = atmo.y;
	if(tAtmoEnter >= tAtmoExit) discard;
    // 相机在大气壳内（进入点为0且来自壳内）：只在贴近星球轮廓的一圈才画，避免铺满全屏
    // if (tAtmoEnter <= 0.001) {
    //     float distToPlanetCenter = length(camPos - planetCenter);
    //     if (distToPlanetCenter > planetRadius * 1.02) {   // 人在壳内但没进星体
    //         // 只保留视线离星球很近的（即轮廓附近），其余丢弃
    //         vec3 closest = planetCenter - camPos;
    //         float impact = length((camPos + rd * max(dot(closest, rd), 0.0)) - planetCenter);
    //         if (impact > planetRadius * 1.15) discard;    // 视线扫不到星球附近 → 不画
    //     }
    // }


	 // 3. 火星球交点（大气被火星本体挡住）
    vec2 pl = raySphere(camPos, rd, planetCenter, planetRadius);
    float tPlanetEnter = pl.x;           // 火星近交点
    float tEnd = tAtmoExit;
    bool hitPlanet = (pl.x > 0.0 && pl.x < tAtmoExit);
    if(hitPlanet) tEnd = min(tEnd, pl.x);   // 大气 march 到火星表面

    // 4. ray-march 大气，积分散射
    int steps = 32;
    float stepLen = (tEnd - tAtmoEnter) / float(steps);
    if(stepLen <= 0.0) discard;

    float pathLen = tEnd - tAtmoEnter;              // 射线在大气壳内的实际路程
    float haloGate = smoothstep(0.5, 1.6, pathLen);  // 盘心短径≈0.5→0；切线长径≈2.5→1

    vec3 scatter = vec3(0.0);
    vec3 transmittance = vec3(1.0);

    for(int i = 0; i < steps; i++)
    {
        float t = tAtmoEnter + (float(i) + 0.5) * stepLen;
        vec3 p = camPos + rd * t;

        // 到大气中心的高度（归一化）
        float h = length(p - planetCenter) / planetRadius - 1.0;   // 0=火星表面, (atmoScale-1)=大气顶
        float hAtmo = max(h, 0.0);
        // 密度随高度指数衰减（大气层，越靠近表面越浓）
        float heightDensity = exp(-hAtmo * falloff) * density;

        // 到太阳的光照（沿太阳方向积分透射率——近似，简洁版只算一束）
        float cosAngle = dot(rd, normalize(sunDir));

        // 瑞利 + 米氏散射
        vec3 rayleigh = rayleighCoef * heightDensity * phaseRayleigh(cosAngle) * (0.15 + 0.65 * haloGate);
        vec3 mie = vec3(miecoef * 1.0, miecoef * 0.9, miecoef * 0.7) * heightDensity * phaseMie(cosAngle) * haloGate;

        vec3 localScatter = (rayleigh + mie) * sunColor * transmittance;
        scatter += localScatter * stepLen;

        // Beer-Lambert 消光
        transmittance *= exp(-(rayleighCoef + vec3(miecoef)) * heightDensity * stepLen);
    }

    // 背光透光晕：视线穿过火星背后的大气
    // 若没有击中火星（射线从火星旁边/背后经过），则把整个大气壳都积分 → 自然有背后透光
    vec3 color = scatter * intensity;

    //float intensity = clamp(dot(color, vec3(0.2126,0.7152,0.0722)), 0.0, 1.0);
    FragColor = vec4(color, 1.0);

}

