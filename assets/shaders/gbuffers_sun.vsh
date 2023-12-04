#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    texCoords = aTexCoord;
    fragPos = vec3(model * vec4(aPos, 1.0f));
    normal = mat3(transpose(inverse(mat3(model)))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}