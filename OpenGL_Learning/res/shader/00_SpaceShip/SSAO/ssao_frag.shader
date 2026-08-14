#version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;
uniform mat4 view;

const int kernelSize = 64;
const float radiusUnify = 2.0;
const float radiusAsteroid = 2.0;
const float radiusPlanet = 8.0;
const float bias = 0.05;
const vec2 noiseScale = vec2(960.0/4.0, 600.0/4.0);

uniform bool unify;

void main()
{
    // 世界空间 G-buffer -> 观察空间
    vec3 fragPosWorld = texture(gPosition, TexCoords).xyz;
    vec3 normalWorld = normalize(texture(gNormal, TexCoords).rgb);
    vec3 fragPos = (view * vec4(fragPosWorld, 1.0)).xyz;
    vec3 normal  = normalize(mat3(view) * normalWorld);

    // 判断radius
    float radius = unify ? radiusUnify : (length(fragPosWorld) < 50.0) ? radiusPlanet : radiusAsteroid;

    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i];       // 切线 -> 观察空间
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // 关键：gPosition 存的是世界空间，必须转观察空间再取 .z 深度
        vec3 sampleWorldPos = texture(gPosition, offset.xy).xyz;
        float sampleDepth = (view * vec4(sampleWorldPos, 1.0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    FragColor = 1.0 - (occlusion / kernelSize);
}