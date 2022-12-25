#version 330 core
in vec3 TexCoords;
in vec3 position;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;
out vec4 FragColor;
uniform samplerCube skyBox;

void main(){
//    FragColor = texture(skyBox, TexCoords);
    gPosition = position;
    gDiffuseSpecular = texture(skyBox, TexCoords);

}