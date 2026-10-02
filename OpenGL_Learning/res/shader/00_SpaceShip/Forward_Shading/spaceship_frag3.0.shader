#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
//in vec3 Tangent;
in vec3 ObjPos;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissionMap;

uniform bool  useMetallicMap;
uniform bool  useRoughnessMap;
uniform bool  useAOMap;
uniform bool  useEmissiveMap;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue;
uniform float emissiveStrength;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// 单点光源（太阳），与场景光照 deferred 一致
uniform vec3  lightPositions[1];
uniform vec3  lightColors[1];
uniform float lightConstant;
uniform float lightLinear;
uniform float lightQuadratic;

uniform vec3 camPos;


// ===== 折跃：两级轴向溶解与白化（计划书 §3.5）=====
uniform float uVanish;         // u_v，整体白化强度
uniform float uWhiteK;         // 白热系数 K
uniform float uFrontMat;       // 材质剥离前沿 = warp::matFront(tau)
uniform float uFrontGeo;       // 几何剔除前沿 = warp::geoFront(tau)
uniform float uAxisMinY;       // 模型局部 Y 的最小值（机头在这一端）
uniform float uAxisMaxY;       // 模型局部 Y 的最大值
uniform float uObjRadius;      // 归一化半径（与外壳 pass 同值）
uniform float uNoiseW;         // 噪声抖动权重 = warp::NOISE_W
uniform float uNoiseFreq;      // 噪声空间频率
uniform float uEdge;           // 切口发光带宽
uniform vec3  uDissolveColor;  // 溶解/合成的效果色
uniform float uFrontRev;       // 0=去程；1=返程坐标镜像

const float PI = 3.14159265359;


// 阴影（接收场景 depthCubeMap）
uniform samplerCube depthMap;
uniform float far_plane;
uniform bool  shadows;
uniform bool  PCSS;
uniform samplerCube depthDynMap;
float ShadowDepthMin(vec3 dir) {
    //return min(texture(depthMap, dir).r, texture(depthDynMap, dir).r);
    return texture(depthMap, dir).r;
}

// 菲涅尔边缘光（rim light）
uniform vec3  rimColor;
uniform float rimPower;
uniform float rimStrength;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

float ShadowCalculationPCF(vec3 fragPos, vec3 normal);
float ShadowCalculationPCSS(vec3 fragPos, vec3 normal);

// 噪声函数：
float hash13(vec3 p);
float noise3(vec3 p);

void main()
{
	// material properties
    vec3 albedo    = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
	// float metallic = useMetallicMap  ? texture(metallicMap,  TexCoords).r : metallicValue;
	// float roughness= useRoughnessMap ? texture(roughnessMap, TexCoords).r : roughnessValue;
	float metallic = useMetallicMap  ? texture(metallicMap,  TexCoords).b : metallicValue;   // glTF ORM: B=metal
	float roughness= useRoughnessMap ? texture(roughnessMap, TexCoords).g : roughnessValue;  // glTF ORM: G=rough
	float ao       = useAOMap        ? texture(aoMap,        TexCoords).r : aoValue;

	// input lighting data
	vec3 N = getNormalFromMap();
	vec3 V = normalize(camPos - WorldPos);

    
    // ===== 折跃：两级轴向溶解 =====
    // 轴向坐标：机头是局部 -Y，所以 y 最小处是艏。g = 0 在艏、1 在艉。
    // 物体空间坐标比世界大 2000 倍，必须先除以半径再喂噪声，否则同一组频率会变成噪点。
    vec3  q  = ObjPos / max(uObjRadius, 1e-3);
    float g  = (ObjPos.y - uAxisMinY) / max(uAxisMaxY - uAxisMinY, 1e-6);

    // 噪声减 0.5 居中：不减的话前沿的平均位置会被整体推偏 uNoiseW/2。
    // float gt = clamp(g + uNoiseW * (noise3(q * uNoiseFreq) - 0.5), 0.0, 1.0);
    // 【不要夹到 [0,1]】夹了之后噪声的负半程会被压成 gt = 0 的一整片平台，
    // 而静止时两条前沿恰好落在 0 附近，smoothstep(0,0,gt) 在 gt = 0 处是 0/0，
    // 那片平台就会被判成"完全剥离"，艏部出现青绿伪影。
    // gt 的取值范围天然是 [-uNoiseW/2, 1 + uNoiseW/2]，两条前沿的取值域已经覆盖它。
    float gt = g + uNoiseW * (noise3(q * uNoiseFreq) - 0.5);
    float h = mix(gt, 1.0 - gt, uFrontRev);

    //if (gt < uFrontGeo) discard;                      // 几何已剔除
    if (h < uFrontGeo) discard;
    // 材质已剥离的程度：1 = 已剥离，0 = 完好。
    // 注意方向：前沿从艏(0)扫向艉(1)，"被扫过"的是 gt 较小的一侧，所以这里是 1 - smoothstep。
    //float stripped = 1.0 - smoothstep(uFrontGeo, uFrontMat, gt);
    float stripped = 1.0 - smoothstep(uFrontGeo, uFrontMat, h);

    // 剥离区：丢掉贴图细节、压暗反照率、拉满粗糙度、换回几何法线。
    albedo    = mix(albedo,    uDissolveColor * 0.15, stripped);
    metallic  = mix(metallic,  0.0,  stripped);
    roughness = mix(roughness, 1.0,  stripped);
    N         = mix(N, normalize(Normal), stripped);

    // 切口发光：边界带上一圈亮，读作被撕裂的白热切口。
    // 两端加窗，否则前沿停在 0 或 1 时艏/艉会凭空亮一圈。
    // float band = 1.0 - smoothstep(0.0, uEdge, abs(gt - uFrontGeo));
    float band = 1.0 - smoothstep(0.0, uEdge, abs(h - uFrontGeo));
    float win  = smoothstep(0.0, 0.05, uFrontGeo) * smoothstep(0.0, 0.05, 1.0 - uFrontGeo);
    vec3  warpGlow = uDissolveColor * band * win;

    
	vec3 R = reflect(-V, N);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	float shadow = shadows ? PCSS ? ShadowCalculationPCSS(WorldPos, N) : ShadowCalculationPCF(WorldPos, N): 0.0;

	// reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < 1; ++i)
	{
		// calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);
		vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

		vec3 numerator   = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;
		
		// scale light by NdotL
		float NdotL = max(dot(N, L), 0.0);

		// add to outgoing radiance Lo
		//Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	    float lightShadow = (i == 0) ? shadow : 0.0;   // 只有太阳(第0个)投阴影
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - lightShadow);
    }

	// ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    // ambient lighting (we now use IBL as the ambient term)

    // IBL
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
	
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * 2.0 * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
	    
	vec3 spaceAmbient = vec3(0.04, 0.06, 0.10);
    
    ambient += spaceAmbient * albedo;

	vec3 color = ambient + Lo;
	
	if (useEmissiveMap)
        color += pow(texture(emissionMap, TexCoords).rgb ,vec3(2.2)) * emissiveStrength;

    // 菲涅尔边缘光：背光轮廓用淡蓝色勾勒（科幻飞船经典 rim light）
	float rim = pow(1.0 - max(dot(N, V), 0.0), rimPower);
	color += rimColor * rim * rimStrength;
    
    color += warpGlow * uVanish;
	// // HDR tonemapping
	// color = color / (color + vec3(1.0));
	// // gamma correct
	// color = pow(color, vec3(1.0/2.2));

	//FragColor = vec4(color, 1.0);
    // 整体白化：u_v 冲到 1 时整艘舰变成 uWhiteK 倍的白，远超 bright pass 阈值、
	// 必然进 bloom。这是"烧成光"那一半；溶解那一半在 ④ 里。
	FragColor = vec4(mix(color, vec3(uWhiteK), uVanish), 1.0);
}

// ----------------------------------------------------------------------------

// ---- 物体空间 3D 值噪声：----
// 不引纹理，也不受"平面投影到船身会拉伸"的影响。
// 输入是归一化后的物体空间坐标（量级约 ±1），所以频率能直接用 O(10) 的常数。
float hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float noise3(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float n000 = hash13(i + vec3(0.0, 0.0, 0.0));
    float n100 = hash13(i + vec3(1.0, 0.0, 0.0));
    float n010 = hash13(i + vec3(0.0, 1.0, 0.0));
    float n110 = hash13(i + vec3(1.0, 1.0, 0.0));
    float n001 = hash13(i + vec3(0.0, 0.0, 1.0));
    float n101 = hash13(i + vec3(1.0, 0.0, 1.0));
    float n011 = hash13(i + vec3(0.0, 1.0, 1.0));
    float n111 = hash13(i + vec3(1.0, 1.0, 1.0));
    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);
    return mix(mix(nx00, nx10, f.y), mix(nx01, nx11, f.y), f.z);
}



// ----------------------------------------------------------------------------
// --- PCF 阴影 ---
float ShadowCalculationPCF(vec3 fragPos, vec3 normal)
{
    fragPos += normal * 0.3;   // ← 加这行：飞船小，用小值
    vec3 fragToLight = fragPos - lightPositions[0];
    float currentDepth = length(fragToLight);
    //float closestDepth = texture(depthMap, fragToLight).r * far_plane;
    float closestDepth = ShadowDepthMin(fragToLight) * far_plane;
    vec3 lightDir = normalize(lightPositions[0] - fragPos);

    // 固定方向种子 (不深究，随机采样会闪，用固定)
    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    float shadow = 0.0;
    // 飞船是单个物体，用正常 bias；不像小行星带那样调到 100（否则自身投影全失效）
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 65);
    float viewDistance = length(camPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for (int i = 0; i < 20; ++i)
    {
        //float d = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r * far_plane;
        float d = ShadowDepthMin(fragToLight + sampleOffsetDirections[i] * diskRadius) * far_plane;
        if (currentDepth - bias > d)
            shadow += 1.0;
    }
    shadow /= 20.0;
    return shadow;
}
// ----------------------------------------------------------------------------

// --- PCSS 阴影 --- 
float ShadowCalculationPCSS(vec3 fragPos, vec3 normal)
{
    vec3 baseDir = normalize(fragPos - lightPositions[0]);
    float receiverDepth = length(fragPos - lightPositions[0]);
    vec3 lightDir = normalize(lightPositions[0] - fragPos);
    float NdotL = dot(normal, lightDir);

    vec3 sampleDirs[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    // ==========================================
    // Stage 1: Blocker Search (with cone-base Z offset)
    // ==========================================
    float lightRadius = 120.0;
    float lightAngSize = atan(lightRadius / receiverDepth);
    float searchAngle = max(lightAngSize * 3.0, 0.03);
    searchAngle = min(searchAngle, 0.15);

    float blockerSum = 0.0;
    int blockerCount = 0;

    for (int i = 0; i < 16; ++i)
    {
        vec3 probeDir = normalize(baseDir + sampleDirs[i] * searchAngle);
        //float probeDepth = texture(depthMap, probeDir).r * far_plane;
        float probeDepth = ShadowDepthMin(probeDir) * far_plane;

        // cone-base Z offset: planar surface at angle appears deeper
        // zOffset = receiverDepth * (1/cos(angle) - 1) 约等于 receiverDepth * angle²/2
        float cosAngle = dot(baseDir, probeDir);
        float zOffset = receiverDepth * (1.0 / max(cosAngle, 0.85) - 1.0);

        if (probeDepth < receiverDepth - zOffset - 2.0)
        {
            blockerSum += probeDepth;
            blockerCount++;
        }
    }

    // fallback: fixed PCF
    if (blockerCount < 4)
    {
        float shadow = 0.0;
        float bias = max(0.05 * (1.0 - NdotL), 0.15);
        float viewDistance = length(camPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

        for (int i = 0; i < 20; ++i)
        {
            vec3 probeDir = normalize(baseDir + sampleDirs[i] * diskRadius);
            //float d = texture(depthMap, probeDir).r * far_plane;
            float d = ShadowDepthMin(probeDir) * far_plane;
            if (receiverDepth - bias > d) shadow += 1.0;
        }
        shadow /= 20.0;
        return shadow;
    }

    float avgBlockerDepth = blockerSum / float(blockerCount);

    // ==========================================
    // Stage 2: Penumbra (0.9 attenuation from HLSL)
    // ==========================================
    float blockerDist = min(abs(avgBlockerDepth - receiverDepth) * 0.9, 400.0);
    float penumbra = atan(lightRadius * blockerDist / (avgBlockerDepth * receiverDepth));
    penumbra = clamp(penumbra, 0.01, 0.15);

    // ==========================================
    // Stage 3: PCF adaptive
    // ==========================================
    float shadow = 0.0;
    float bias = max(0.05 * (1.0 - NdotL), 0.15);

    for (int i = 0; i < 20; ++i)
    {
        vec3 probeDir = normalize(baseDir + sampleDirs[i] * penumbra);
        //float d = texture(depthMap, probeDir).r * far_plane;
        float d = ShadowDepthMin(probeDir) * far_plane;
        if (receiverDepth - bias > d) shadow += 1.0;
    }
    shadow /= 20.0;

    return shadow;
}

// ----------------------------------------------------------------------------
// 法线获取函数，包含TBN计算
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
	// vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

 //    vec3 N = normalize(Normal);
 //    vec3 T = normalize(Tangent);
 //    T = normalize(T - dot(T, N) * N);   // Gram-Schmidt：让切线垂直于法线
 //    vec3 B = normalize(cross(N, T));
 //    mat3 TBN = mat3(T, B, N);

 //    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N,V), 0.0);
	float NdotL = max(dot(N,L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------