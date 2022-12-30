#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpecular;

//out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

uniform int isTree;

vec3 phong() {
    vec3 objectColor = vec3(texture(texture_diffuse1, fs_in.TexCoords));
    //  环境光照：将环境光强度乘以光的颜色，就可以得到环境光的颜色
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    //  漫反射
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(-lightDirection);  //光线的方向向量，将光源位置向量与片元位置向量相减就可以得到
    float diff = max(dot(norm, lightDir), 0.0);     //点积，获得cos值
    vec3 diffuse = diff * lightColor;

    //  镜面反射
    float specularStrength = 1.0f;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);    //摄像机的观察方向向量，将摄像机位置向量与片元位置向量相减就可以得到
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(texture(texture_specular1, fs_in.TexCoords));

    //  合成
    vec3 result = (ambient + diffuse + specular) * objectColor;
    return result;
}

void main() {
    //    vec4 color = vec4(phong(), 1.0);
    vec4 tex = texture(texture_diffuse1, fs_in.TexCoords);
    if (isTree == 1 && tex.a < 1.0) discard;
    //    FragColor = color;
    gPosition = fs_in.FragPos;
    gNormal = normalize(fs_in.Normal);
    gDiffuseSpecular.rgb = tex.rgb;
    gDiffuseSpecular.a = texture(texture_specular1, fs_in.TexCoords).r;
}