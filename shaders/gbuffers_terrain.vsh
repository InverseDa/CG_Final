#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;
in vec3 color;

//out float Height;
//out vec3 aColor;
//out vec3 position;
out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform sampler2D normal;

void main(){
    vec3 norm = texture(normal, texCoord).rgb;
    vec3 position = vec3(aPos.x, aPos.y, aPos.z);
//    Height = aPos.y;
//    aColor = color;
    vs_out.FragPos = vec3(model * vec4(position, 1.0f));
    vs_out.Normal = norm;
    vs_out.TexCoords = texCoord;
//    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0f);
    gl_Position = projection * view * model * vec4(position, 1.0);
}