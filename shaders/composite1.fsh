#version 330 core

const int bottom = 250;
const int top = 300;
const int width = 1000;

const vec3 baseBright = vec3(1.26, 1.25, 1.29);    // 基础颜色 -- 亮部
const vec3 baseDark = vec3(0.31, 0.31, 0.32);   // 基础颜色 -- 暗部
const vec3 lightBright = vec3(1.29, 1.17, 1.05); // 光照颜色 -- 亮部
const vec3 lightDark = vec3(0.7, 0.75, 0.8);   // 光照颜色 -- 暗部

const int noiseTextureResolution = 128;

in vec2 TexCoords;

layout (location = 0) out vec3 comPosition;
layout (location = 1) out vec3 comNormal;
layout (location = 2) out vec4 comDiffuseSpecular;
layout (location = 3) out vec4 comWaterTex;

out vec4 fcolor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuseSpecular;
uniform sampler2D gDepthTex;
uniform sampler2D gWaterTex;
uniform sampler2D shadowMap;
uniform sampler2D noisetex;

uniform mat4 projection;
uniform mat4 projectionInverse;
uniform mat4 view;
uniform mat4 viewInverse;
uniform mat4 lightSpaceMatrix;

uniform float near;
uniform float far;
uniform float worldTime;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

// 计算 pos 点的云密度
float getDensity(vec3 pos) {
    // 高度衰减
    float mid = (bottom + top) / 2.0;
    float h = top - bottom;
    float weight = 1.0 - 2.0 * abs(mid - pos.y) / h;
    weight = pow(weight, 0.5);

    // 采样噪声图
    vec2 coord = pos.xz * 0.0010;
    float noise = texture(noisetex, coord).x;
    noise += texture(noisetex, coord * 3.5).x / 3.5;
    noise += texture(noisetex, coord * 12.5).x / 12.5;
    noise += texture(noisetex, coord * 42.87).x / 42.87;
    noise /= 1.4472;
    noise *= weight;

    // 截断
    if (noise < 0.4) {
        noise = 0;
    }

    return noise;
}

vec4 getCloud(vec3 fragPos) {
    vec3 dir = normalize(fragPos - viewPos);
    vec3 step = dir * 0.25;
    vec4 colorSum = vec4(0.0f);
    vec3 point = viewPos;

    // 如果相机在云层下，将测试起始点移动到云层底部 bottom
    if (point.y < bottom) {
        point += dir * (abs(bottom - viewPos.y) / abs(dir.y));
    }
    // 如果相机在云层上，将测试起始点移动到云层顶部 top
    if (top < point.y) {
        point += dir * (abs(viewPos.y - top) / abs(dir.y));
    }

    // 如果目标像素遮挡了云层则放弃测试
    float len1 = length(point - viewPos);     // 云层到眼距离
    float len2 = length(fragPos - viewPos); // 目标像素到眼距离
    if (len2 < len1) {
        return vec4(0);
    }

    for (int i = 0; i < 100; i++) {
        point += step;
        if (bottom > point.y || point.y > top || -width > point.x ||
        point.x > width || -width > point.z || point.z > width) {
            continue;
        }
        float density = getDensity(point);
        vec3 L = normalize(lightPos - point);                       // 光源方向
        float lightDensity = getDensity(point + L);       // 向光源方向采样一次 获取密度
        float delta = clamp(density - lightDensity, 0.0, 1.0);      // 两次采样密度差

        // 控制透明度
        density *= 0.5;

        // 颜色计算
        vec3 base = mix(baseBright, baseDark, density) * density;   // 基础颜色
        vec3 light = mix(lightDark, lightBright, delta);            // 光照对颜色影响

        // 混合
        vec4 color = vec4(base * light, density);                     // 当前点的最终颜色
        colorSum = color * (1.0 - colorSum.a) + colorSum;           // 与累积的颜色混合
    }
    return colorSum;
}

float ShadowCalculation(vec4 fragPos) {
    //    float bias = max(0.05 * (1.0 - dot(texture(normal, fs_in.TexCoords).rgb, -lightDirection)), 0.005);
    vec4 fragPosLightSpace = lightSpaceMatrix * fragPos;
    float bias = 0.005;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closetDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.0002 > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}

vec3 phong(vec3 Diffuse, vec3 FragPos, vec3 Normal, float Specular) {
    //  环境光照：将环境光强度乘以光的颜色，就可以得到环境光的颜色
    float ambientStrength = 1.0f;
    vec3 ambient = ambientStrength * lightColor;

    //  漫反射
    vec3 lightDir = normalize(-lightDirection);  //光线的方向向量，将光源位置向量与片元位置向量相减就可以得到
    float diff = max(dot(Normal, lightDir), 0.0);     //点积，获得cos值
    vec3 diffuse = diff * lightColor;

    //  镜面反射
    float specularStrength = 1.0f;
    vec3 viewDir = normalize(viewPos - FragPos);    //摄像机的观察方向向量，将摄像机位置向量与片元位置向量相减就可以得到
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * texture(gDiffuseSpecular, TexCoords).rgb;

    float shadow = ShadowCalculation(vec4(FragPos, 1.0));
    //  合成
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * Diffuse;
    return result;
}

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float getWave(vec3 pos) {
    float speed1 = worldTime * 85 / (noiseTextureResolution * 15);
    vec3 coord1 = pos.xyz / noiseTextureResolution;
    coord1.x *= 3;
    coord1.x += speed1;
    coord1.z += speed1 * 0.2;
    float noise1 = texture(noisetex, coord1.xz).x;

    float speed2 = worldTime * 85 / (noiseTextureResolution * 7);
    vec3 coord2 = pos.xyz / noiseTextureResolution;
    coord2.x *= 0.5;
    coord2.x -= speed2 * 0.15 + noise1 * 0.05;  // 加入第一个波浪的噪声
    coord2.z -= speed2 * 0.7 - noise1 * 0.05;
    float noise2 = texture(noisetex, coord2.xz).x;

    return noise2 * 0.6 + 0.4;;
}

// Raymarch algorithm, return uv coordinates(vec2)
vec2 rayMarch(vec3 reflPosition, vec3 reflDir) {
    int steps = 30;
    vec3 reflPos = reflPosition;
    for (int i = 0; i < steps; i++) {
        reflPos += reflDir * 5;
        vec4 UV = projection * vec4(reflPos, 1.0);
        UV.xyz /= UV.w;
        UV.xyz = UV.xyz * 0.5 + 0.5;
        if (UV.x < 0 || UV.x > 1 ||
        UV.y < 0 || UV.y > 1)
        break;
        float worldDepth = linearizeDepth(texture(gDepthTex, UV.st).r);
        float curDepth = linearizeDepth(UV.z);
        if (abs(worldDepth) - abs(curDepth) <= 0) {
            return UV.st;
        }
    }
    return vec2(0.0);
}

void main()
{
    // retrieve data from gbuffer
    // world position
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    // DiffuseColor
    vec3 Diffuse = texture(gDiffuseSpecular, TexCoords).rgb;
    // feature map
    vec4 water = texture(gWaterTex, TexCoords);
    // specular
    float Specular = texture(gDiffuseSpecular, TexCoords).a;
    // world normal map
    vec3 Normal = texture(gNormal, TexCoords).rgb;          //[0,1]
    Normal = normalize(2.0 * Normal - 1.0);                 //[-1,1]

    Diffuse = phong(Diffuse, FragPos, Normal, Specular);
    comDiffuseSpecular = vec4(Diffuse, 1.0f);
    comNormal = texture(gNormal, TexCoords).rgb;
    comPosition = FragPos;
    comWaterTex = water;
}