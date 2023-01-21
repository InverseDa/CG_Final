#version 330 core
const int noiseTextureResolution = 128;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuseSpecular;
uniform sampler2D gDepthTex;
uniform sampler2D waterTex;
uniform sampler2D shadowMap;
uniform sampler2D noisetex;

uniform mat4 projection;
uniform mat4 projectionInverse;
uniform mat4 view;
uniform mat4 viewInverse;
uniform mat4 lightSpaceMatrix;

uniform float near;
uniform float far;
uniform float worldTime;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float getWave(vec3 pos) {
    float speed1 = worldTime * 85 / (noiseTextureResolution * 15);
    vec3 coord1 = pos.xyz / noiseTextureResolution;
    coord1.x *= 3;
    coord1.x += speed1;
    coord1.z += speed1 * 0.2;
    float noise1 = texture(noisetex, coord1.xz).x;

    float speed2 = worldTime * 85 / (noiseTextureResolution * 7);
    vec3 coord2 = pos.xyz / noiseTextureResolution;
    coord2.x *= 0.5;
    coord2.x -= speed2 * 0.15 + noise1 * 0.05;  // 加入第一个波浪的噪声
    coord2.z -= speed2 * 0.7 - noise1 * 0.05;
    float noise2 = texture(noisetex, coord2.xz).x;

    return noise2 * 0.6 + 0.4;;
}

// Raymarch algorithm, return uv coordinates(vec2)
vec2 rayMarch(vec3 reflPosition, vec3 reflDir) {
    int steps = 30;
    vec3 reflPos = reflPosition;
    for (int i = 0; i < steps; i++) {
        reflPos += reflDir * 0.2;
        vec4 UV = projection * vec4(reflPos, 1.0);
        UV.xyz /= UV.w;
        UV.xyz = UV.xyz * 0.5 + 0.5;
        if (UV.x < 0 || UV.x > 1 ||
        UV.y < 0 || UV.y > 1)
        break;
        float worldDepth = linearizeDepth(texture(gDepthTex, UV.st).r);
        float curDepth = linearizeDepth(UV.z);
        if (abs(worldDepth) - abs(curDepth) <= 0) {
            return UV.st;
        }
    }
    return vec2(0.0);
}

void main()
{
    // DiffuseColor
    vec3 Diffuse = texture(gDiffuseSpecular, TexCoords).rgb;
    // feature map
    vec4 water = texture(waterTex, TexCoords);
    // specular
    float Specular = texture(gDiffuseSpecular, TexCoords).a;
    // world normal map
    vec3 Normal = vec3(0, 1, 0);
    // depth
    float depth = texture(gDepthTex, TexCoords).r;
    vec3 FragPos = vec3(TexCoords.st * 2.0 - 1.0, depth * 2.0 - 1.0);
    vec3 waterColor = vec3(0.0);
    if (Diffuse == vec3(0.0)) {
        vec4 positionInView = view * vec4(FragPos, 1.0);
        vec3 vDir = normalize(positionInView.xyz);
        vec3 norm = mat3(view) * Normal;
        vec3 reflectDir = normalize(reflect(vDir, norm));
        vec2 uv = rayMarch(positionInView.xyz, reflectDir);
        if(uv != vec2(0.0))
        waterColor = texture(gDiffuseSpecular, uv).rgb;
        //finalColor = mix(Diffuse, finalColor, 0.5);
    }
    FragColor = vec4(Diffuse + water.rgb, 1.0);
}