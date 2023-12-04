#version 330 core
in vec3 position;
in vec3 normal;
in vec3 color;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;

void main() {
    gPosition = position;
    gNormal = normalize(normal);
    gDiffuseSpecular.rgb = color.rgb;
    gDiffuseSpecular.a = 1.0f;
}