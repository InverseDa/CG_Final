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


//float noise(vec3 x)
//{
//    vec3 p = floor(x);
//    vec3 f = fract(x);
//    f = smoothstep(0.0, 1.0, f);
//
//    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
//    float v1 = texture(noisetex, (uv) / 256.0, -100.0).x;
//    float v2 = texture(noisetex, (uv + vec2(37.0, 17.0)) / 256.0, -100.0).x;
//    return mix(v1, v2, f.z);
//}
//
//float getCloudNoise(vec3 worldPos) {
//    vec3 coord = worldPos;
//    coord *= 0.2;
//    float n = noise(coord) * 0.5; coord *= 3.0;
//    n += noise(coord) * 0.25; coord *= 3.01;
//    n += noise(coord) * 0.125; coord *= 3.02;
//    n += noise(coord) * 0.0625;
//    return max(n - 0.5, 0.0) * (1.0 / (1.0 - 0.5));
//}
//
//// 计算 pos 点的云密度
//float getDensity(vec3 pos) {
//    // 高度衰减
//    float mid = (bottom + top) / 2.0;
//    float h = top - bottom;
//    float weight = 1.0 - 2.0 * abs(mid - pos.y) / h;
//    weight = pow(weight, 0.5);
//
//    // 采样噪声图
//    vec2 coord = pos.xz * 0.0010;
//    float noise = texture(noisetex, coord).x;
//    noise += texture(noisetex, coord * 3.5).x / 3.5;
//    noise += texture(noisetex, coord * 12.5).x / 12.5;
//    noise += texture(noisetex, coord * 42.87).x / 42.87;
//    noise /= 1.4472;
//    noise *= weight;
//
//    // 截断
//    if (noise < 0.4) {
//        noise = 0;
//    }
//
//    return noise;
//}
//
//vec4 getCloud() {
//    vec3 dir = normalize(fs_in.FragPos - viewPos);
//    vec3 step = dir * 0.25;
//    vec4 colorSum = vec4(0.0f);
//    vec3 point = viewPos;
//
//    // 如果相机在云层下，将测试起始点移动到云层底部 bottom
//    if (point.y < bottom) {
//        point += dir * (abs(bottom - viewPos.y) / abs(dir.y));
//    }
//    // 如果相机在云层上，将测试起始点移动到云层顶部 top
//    if (top < point.y) {
//        point += dir * (abs(viewPos.y - top) / abs(dir.y));
//    }
//
//    // 如果目标像素遮挡了云层则放弃测试
//    float len1 = length(point - viewPos);     // 云层到眼距离
//    float len2 = length(fs_in.FragPos - viewPos); // 目标像素到眼距离
//    if (len2 < len1) {
//        return vec4(0);
//    }
//
//    for (int i = 0; i < 100; i++) {
//        point += step;
//        if (bottom > point.y || point.y > top || -width > point.x ||
//        point.x > width || -width > point.z || point.z > width) {
//            continue;
//        }
//        float density = getDensity(point);
//        vec3 L = normalize(lightPos - point);                       // 光源方向
//        float lightDensity = getDensity(point + L);       // 向光源方向采样一次 获取密度
//        float delta = clamp(density - lightDensity, 0.0, 1.0);      // 两次采样密度差
//
//        // 控制透明度
//        density *= 0.5;
//
//        // 颜色计算
//        vec3 base = mix(baseBright, baseDark, density) * density;   // 基础颜色
//        vec3 light = mix(lightDark, lightBright, delta);            // 光照对颜色影响
//
//        // 混合
//        vec4 color = vec4(base * light, density);                     // 当前点的最终颜色
//        colorSum = color * (1.0 - colorSum.a) + colorSum;           // 与累积的颜色混合
//    }
//    return colorSum;
//}
//
//float ShadowCalculation(vec4 fragPosLightSpace) {
////    float bias = max(0.05 * (1.0 - dot(texture(normal, fs_in.TexCoords).rgb, -lightDirection)), 0.005);
//    float bias = 0.005;
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closetDepth = texture(shadowMap, projCoords.xy).r;
//    float currentDepth = projCoords.z;
//    // PCF
//    float shadow = 0.0;
//    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//            shadow += currentDepth > pcfDepth  ? 1.0 : 0.0;
//        }
//    }
//    shadow /= 9.0;
//
//    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
//    if(projCoords.z > 1.0)
//    shadow = 0.0;
//
//    return shadow;
////    return currentDepth > closetDepth ? 1.0 : 0.0;
//}
//
//vec3 phong(vec3 objectColor) {
//    //  环境光照：将环境光强度乘以光的颜色，就可以得到环境光的颜色
//    float ambientStrength = 1.0f;
//    vec3 ambient = ambientStrength * lightColor;
//
//    //  漫反射
//    vec3 norm = texture(normal, fs_in.TexCoords).rgb;
//    norm = normalize(norm * 2.0 - 1.0);
//    vec3 lightDir = normalize(-lightDirection);  //光线的方向向量，将光源位置向量与片元位置向量相减就可以得到
//    float diff = max(dot(norm, lightDir), 0.0);     //点积，获得cos值
//    vec3 diffuse = diff * lightColor;
//
//    //  镜面反射
//    float specularStrength = 1.0f;
//    vec3 viewDir = normalize(viewPos - fs_in.FragPos);    //摄像机的观察方向向量，将摄像机位置向量与片元位置向量相减就可以得到
//    vec3 reflectDir = reflect(-lightDir, norm);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//    vec3 specular = specularStrength * spec * texture(specular, fs_in.TexCoords).rgb;
//
//    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
//    //  合成
//    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;
//    return result;
//}

void main() {
//    vec4 cloud = getCloud();
//    vec3 color = phong(texture(tex, fs_in.TexCoords).rgb);
//    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
//    vec3 norm = normalize(texture(normal, fs_in.TexCoords).rgb * 2.0 - 1.0);
//    float isUnder = dot(normalize(lightDirection), normalize(-norm));
//    if(shadow == 1.0 || isUnder < 0.0f) {
//        FragColor.rgb = 0.7 * (color * (1.0 - cloud.a) + cloud.rgb);
//    }
//    else {
//        FragColor.rgb = (color * (1.0 - cloud.a) + cloud.rgb);
//    }
    vec3 norm = texture(normal, fs_in.TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    gPosition = fs_in.FragPos;
    gNormal = normalize(norm);
    gDiffuseSpecular.rgb = texture(tex, fs_in.TexCoords).rgb;
    gDiffuseSpecular.a = texture(specular, fs_in.TexCoords).r;
    gFeatureTex = vec4(0.9);
}