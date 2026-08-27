#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

// --- G-Buffer ---
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;   // rgb=albedo, a=metallic
uniform sampler2D gPBR;      // r=roughness, g=ao, a=emission
uniform sampler2D ssao;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

uniform vec3 viewPos;

// 菲涅尔
uniform vec3  rimColor;
uniform float rimPower;
uniform float rimStrength;


// --- Point Light ---
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];



uniform float shininess;

// --- Shadows ---
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform float far_plane;
uniform bool shadows;
uniform bool PCSS;
uniform bool ssaoEnabled;

// --- Shadows Func --- 
float ShadowCalculation(vec3 fragPos, vec3 normal);
float ShadowCalculationPCSS(vec3 fragPos, vec3 normal);


const float PI = 3.14159265359;

// === PBR BRDF ===
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float spec);
//vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float spec, float shadow);



void main()
{
    vec3 FragPos   = texture(gPosition, TexCoords).rgb;
    vec3 Normal    = texture(gNormal, TexCoords).rgb;
    vec3 albedo    = texture(gAlbedo, TexCoords).rgb;
    float metallic = texture(gAlbedo, TexCoords).a;
    vec4 pbr       = texture(gPBR, TexCoords);
    float roughness= pbr.r;
    float ao       = pbr.g;
    float emission = pbr.a;

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 R = reflect(-V, N);

    //float shadow = shadows ? ShadowCalculation(FragPos, norm) : 0.0;
    float shadow = shadows ? PCSS ? ShadowCalculationPCSS(FragPos, N) : ShadowCalculation(FragPos, N) : 0.0;
    

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        vec3 L = normalize(pointLights[i].position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant
                                 + pointLights[i].linear * distance
                                 + pointLights[i].quadratic * distance * distance);
        vec3 radiance = pointLights[i].color * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        
        float NdotL = max(dot(N, L), 0.0);
        vec3 specular = (NDF * G * F) / max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.0001);

        float lightShadow = (i == 0) ? shadow : 0.0;   // 只有太阳(第0个)投阴影
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - lightShadow);
    }

    //IBL
    vec3 F  = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * 2.0 * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    // 太空背光面 irradiance 趋近于 0，会死黑；加一层极微弱星野环境光（深蓝）让暗面可读
    vec3 spaceAmbient = vec3(0.02, 0.03, 0.05);
    
    ambient += spaceAmbient * albedo * (1.0 - metallic);

    float ssaoFactor = ssaoEnabled ? texture(ssao, TexCoords).r : 1.0;

    vec3 color = ambient * ssaoFactor + Lo;
    color += albedo * emission;

    // 菲涅尔边缘光：小行星轮廓
    vec3 Lsun = normalize(pointLights[0].position - FragPos);
    float rim = pow(1.0 - max(dot(N, V), 0.0), rimPower);
    //color += rimColor * rim * rimStrength;
    color += rimColor * rim * rimStrength * max(dot(N, Lsun), 0.0);

    FragColor = vec4(color, 1.0);
}

// // --- 定向光 ---
// vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float spec)
// {
//     vec3 lightDir = normalize(-light.direction);
//     float diff = max(dot(normal, lightDir), 0.0);
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float specVal = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

//     float ao = ssaoEnabled ? texture(ssao, TexCoords).r : 1.0;

//     vec3 ambient  = light.ambient * albedo * ao;
//     vec3 diffuse  = light.diffuse * diff * albedo;
//     vec3 specular = light.specular * specVal * spec;

//     return ambient + diffuse + specular;
// }

// // --- 点光源 ---
// vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float spec, float shadow)
// {
//     vec3 lightDir = normalize(light.position - fragPos);
//     float diff = max(dot(normal, lightDir), 0.0);
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float specVal = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

//     float distance = length(light.position - fragPos);
//     float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

//     float ao = ssaoEnabled ? texture(ssao, TexCoords).r : 1.0;

//     vec3 ambient  = light.ambient * albedo * ao;
//     vec3 diffuse  = light.diffuse * diff * albedo;
//     vec3 specular = light.specular * specVal * spec;

//     ambient  *= attenuation;
//     diffuse  *= attenuation;
//     specular *= attenuation;

//     return ambient + (1.0 - shadow) * (diffuse + specular);
// }

// --- PCF 阴影 ---
float ShadowCalculation(vec3 fragPos, vec3 normal)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= far_plane;

    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );

    float shadow = 0.0;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 100);
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for (int i = 0; i < 20; ++i)
    {
        float d = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        d *= far_plane;
        if (currentDepth - bias > d)
            shadow += 1.0;
    }
    shadow /= 20.0;

    return shadow;
}


// --- PCSS 阴影 --- 
float ShadowCalculationPCSS(vec3 fragPos, vec3 normal)
{
    vec3 baseDir = normalize(fragPos - lightPos);
    float receiverDepth = length(fragPos - lightPos);
    vec3 lightDir = normalize(lightPos - fragPos);
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
        float probeDepth = texture(depthMap, probeDir).r * far_plane;

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
        float viewDistance = length(viewPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

        for (int i = 0; i < 20; ++i)
        {
            vec3 probeDir = normalize(baseDir + sampleDirs[i] * diskRadius);
            float d = texture(depthMap, probeDir).r * far_plane;
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
        float d = texture(depthMap, probeDir).r * far_plane;
        if (receiverDepth - bias > d) shadow += 1.0;
    }
    shadow /= 20.0;

    return shadow;
}


// === PBR BRDF ===
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
