#version 330 core
in vec2 texCoords;
in vec3 normal;
in vec3 fragPos;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform float viewWidth;
uniform float viewHeight;

vec4 getBloomColor(vec4 color) {
    float brightness = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
    if (brightness < 0.5) color.rgb = vec3(0);
    return color;
}

vec3 getBloom() {
    int radius = 15;
    vec3 sum = vec3(0);
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            vec2 offset = vec2(i / viewWidth, j / viewHeight);
            sum += getBloomColor(texture(texture_diffuse1, texCoords.st + offset)).rgb;
        }
    }
    sum /= pow(radius + 1, 2);
    return sum * 0.3;
}

void main() {
//    vec4 color = texture(texture_diffuse1, texCoords);
    //    color.rgb += getBloom();
//    FragColor = color;
    gPosition = fragPos;
    gNormal = normalize(normal);
    gDiffuseSpecular.rgb = texture(texture_diffuse1, texCoords).rgb;
    gDiffuseSpecular.a = texture(texture_specular1, texCoords).r;
}