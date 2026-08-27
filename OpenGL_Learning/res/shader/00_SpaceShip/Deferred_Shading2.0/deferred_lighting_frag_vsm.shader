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

// --- Shadows (VSM) ---
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform float far_plane;
uniform bool shadows;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float spec);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float spec, float shadow);

void main()
{
    vec3 FragPos   = texture(gPosition, TexCoords).rgb;
    vec3 Normal    = texture(gNormal, TexCoords).rgb;
    vec3 Albedo    = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // --- VSM Shadow ---
    float shadow = 0.0;
    if (shadows)
    {
        vec3 fragToLight = FragPos - lightPos;
        float receiverDepth = length(fragToLight) / far_plane;
        float bias = 0.0002 * tan(acos(clamp(dot(norm, normalize(lightPos - FragPos)), 0.0, 1.0)));
        bias = clamp(bias, 0.0, 0.005);

        vec2 moments = texture(depthMap, fragToLight).rg;
        float mu = moments.r;
        float sigma2 = max(moments.g - mu * mu, 0.0);

        float depthDiff = receiverDepth - mu - bias;
        float shadowValue = 1.0;
        if (depthDiff > 0.0)
        {
            shadowValue = sigma2 / (sigma2 + depthDiff * depthDiff);
            shadowValue = clamp(shadowValue, 0.0, 1.0);
        }
        shadow = 1.0 - shadowValue;
        //shadow = 0.0;
    }

    vec3 result = CalcDirLight(dirLight, norm, viewDir, Albedo, Specular);
    result += CalcPointLight(pointLights[0], norm, FragPos, viewDir, Albedo, Specular, shadow);

    FragColor = vec4(result, 1.0);
}

// --- Directional Light ---
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

// --- Point Light ---
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