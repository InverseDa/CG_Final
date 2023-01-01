#version 330 core

const int noiseTextureResolution = 128;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 positionInViewCoord;
} fs_in;
in vec4 clipSpace;
//out vec4 FragColor;

uniform sampler2D waterTexture;
uniform sampler2D cameraDepthMap;
uniform sampler2D noisetex;
uniform samplerCube skyBox;
uniform sampler2D shadowMap;
uniform sampler2D tex;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float worldTime;
uniform float viewHeight;
uniform float viewWidth;
uniform mat4 projection;

float getWave(vec3 pos) {
    float speed1 = worldTime * 85 / (noiseTextureResolution * 15);
    vec3 coord1 = pos.xyz / noiseTextureResolution;
    coord1.x *= 3;
    coord1.x += speed1;
    coord1.z += speed1 * 0.2;
    float noise1 = texture(noisetex, coord1.xz).x;

    // 混合波浪
    float speed2 = worldTime * 85 / (noiseTextureResolution * 7);
    vec3 coord2 = pos.xyz / noiseTextureResolution;
    coord2.x *= 0.5;
    coord2.x -= speed2 * 0.15 + noise1 * 0.05;  // 加入第一个波浪的噪声
    coord2.z -= speed2 * 0.7 - noise1 * 0.05;
    float noise2 = texture(noisetex, coord2.xz).x;

    return noise2 * 0.6 + 0.4;;
}

vec4 reflectSky(float wave) {
    vec3 newNormal = fs_in.Normal;
    newNormal.z += 0.05 * (((wave - 0.4) / 0.6) * 2 - 1);
    newNormal = normalize(newNormal);

    float ratio = 1.00 / 1.33;
    vec3 I = normalize(fs_in.FragPos - viewPos);
    vec3 R = reflect(I, normalize(newNormal));
    vec4 skyColor = vec4(texture(skyBox, R).rgb, 1.0);
    return skyColor;
}

vec4 reflectTerrain(float wave) {
    vec3 newNormal = fs_in.Normal;
    newNormal.z += 0.05 * (((wave - 0.4) / 0.6) * 2 - 1);
    newNormal = normalize(newNormal);

    float ratio = 1.00 / 1.33;
    vec3 I = normalize(fs_in.FragPos - viewPos);
    vec3 R = reflect(I, normalize(newNormal));

    vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;
    vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);
    vec4 terrainColor = vec4(texture(tex, R.xz).rgb, 1.0);
    return terrainColor;
}

vec4 getBloomColor(vec4 color) {
    float brightness = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
    if(brightness < 0.5) color.rgb = vec3(0);
    return color;
}

vec3 getBloom(vec3 color) {
    int radius = 0;
    vec3 sum = vec3(0);
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            vec2 offset = vec2(i / viewWidth, j /viewHeight);
            sum += getBloomColor(vec4(color, 1.0)).rgb;
        }
    }
    sum /= pow(radius + 1, 2);
    return sum * 0.3;
}

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near) / (far + near - z * (far - near));
}

float getDepth(vec4 positionInCameraView) {
    vec3 projCoords = positionInCameraView.xyz / positionInCameraView.w;
    projCoords = projCoords * 0.5 + 0.5;
    float depth = texture(cameraDepthMap, projCoords.xy).r;
    return depth;
}

void main() {
//    vec3 tex = vec3(texture(waterTexture, fs_in.TexCoords));
    vec3 tex = vec3(0.0, 0.8, 1.0);
    float wave = getWave(fs_in.FragPos);
    vec3 color = tex * wave;
    vec4 skyColor = reflectSky(wave);
    vec4 terrainColor = reflectTerrain(wave);

//    vec3 final = vec3(mix(skyColor.rgb, terrainColor.rgb, 0.5));
    vec3 final = vec3(skyColor.rgb);
    final += getBloom(final) * 0.8;
//    FragColor = vec4(final, 0.8);
    gDiffuseSpecular = vec4(final, 0.7);
    gNormal = vec3(0.0, 1.0, 1.0);
    gPosition = fs_in.FragPos;
}