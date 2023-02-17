#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;
layout (location = 3) out vec4 gFeatureTex;

const int bottom = 250;
const int top = 300;
const int width = 1000;

const vec3 baseBright = vec3(1.26, 1.25, 1.29);    // 基础颜色 -- 亮部
const vec3 baseDark = vec3(0.31, 0.31, 0.32);   // 基础颜色 -- 暗部
const vec3 lightBright = vec3(1.29, 1.17, 1.05); // 光照颜色 -- 亮部
const vec3 lightDark = vec3(0.7, 0.75, 0.8);   // 光照颜色 -- 暗部

//in float Height;
//in vec3 aColor;
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

//out vec4 FragColor;

uniform sampler2D shadowMap;
uniform sampler2D noisetex;
uniform sampler2D watertex;
uniform sampler2D tex;
uniform sampler2D specular;
uniform sampler2D normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

uniform mat4 model;

void main() {
    gPosition = fs_in.FragPos;
    gNormal = fs_in.Normal;
    gDiffuseSpecular.rgb = texture(tex, fs_in.TexCoords).rgb;
    gDiffuseSpecular.a = texture(specular, fs_in.TexCoords).r;
//    gFeatureTex = vec4(0.9);
}