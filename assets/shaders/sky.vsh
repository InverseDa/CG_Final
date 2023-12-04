#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 position;
uniform mat4 view;
uniform mat4 projection;

void main(){
    TexCoords = aPos;
    position = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}