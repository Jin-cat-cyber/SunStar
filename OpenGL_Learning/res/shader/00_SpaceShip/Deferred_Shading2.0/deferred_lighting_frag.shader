#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

// --- G-Buffer ---
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// --- Directional Light ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// --- Point Light ---
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform float shininess;

// --- Shadows ---
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform float far_plane;
uniform bool shadows;
uniform bool PCSS;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float spec);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float spec, float shadow);
float ShadowCalculation(vec3 fragPos, vec3 normal);
float ShadowCalculationPCSS(vec3 fragPos, vec3 normal);

void main()
{
    vec3 FragPos   = texture(gPosition, TexCoords).rgb;
    vec3 Normal    = texture(gNormal, TexCoords).rgb;
    vec3 Albedo    = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    //float shadow = shadows ? ShadowCalculation(FragPos, norm) : 0.0;
    float shadow = shadows ? PCSS ? ShadowCalculationPCSS(FragPos, norm) : ShadowCalculation(FragPos, norm) : 0.0;
    

    vec3 result = CalcDirLight(dirLight, norm, viewDir, Albedo, Specular);
    result += CalcPointLight(pointLights[0], norm, FragPos, viewDir, Albedo, Specular, shadow);

    FragColor = vec4(result, 1.0);
}

// --- 定向光 ---
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float spec)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specVal = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * specVal * spec;

    return ambient + diffuse + specular;
}

// --- 点光源 ---
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float spec, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specVal = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * specVal * spec;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

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