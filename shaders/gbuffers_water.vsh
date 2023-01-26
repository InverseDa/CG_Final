#version 330 core
in vec3 aPos;
in vec2 texCoord;

out vec2 aTexcoord;
out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 positionInViewCoord;
}vs_out;
out vec4 clipSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float worldTime;

void main() {
    // getBump
    vec4 worldPosition = model * vec4(aPos, 1.0f);
//    worldPosition.y += 0.8 * (sin(worldPosition.z * 4 + worldTime * 1.5)  + cos(worldPosition.x * 4 + worldTime * 1.5));
    vs_out.FragPos = worldPosition.xyz;
    vs_out.positionInViewCoord = projection * worldPosition;
    vs_out.Normal = vec3(0, 1, 0);
    vs_out.TexCoords = texCoord;
    gl_Position = projection *  view * worldPosition;
    clipSpace = projection *  view * worldPosition;
}