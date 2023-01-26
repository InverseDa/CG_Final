#version 330 core
const int noiseTextureResolution = 128;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D DiffuseSpecular;
uniform sampler2D DepthTex;
uniform sampler2D WaterTex;
uniform sampler2D noisetex;

uniform mat4 projection;            // p
uniform mat4 inverseP;              // p^{-1}
uniform mat4 view;                  // v
uniform mat4 inverseV;              // v^{-1}
uniform mat4 inverseVP;             // (p*v)^{-1}=v^{-1}*p^{-1}
uniform mat4 lightSpaceMatrix;      // lp*lv

uniform float near;
uniform float far;
uniform float worldTime;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;

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
    int steps = 20;
    vec3 reflPos = reflPosition;
    for (int i = 0; i < steps; i++) {
        reflPos += reflDir * 1.75;
        vec4 UV = projection * vec4(reflPos, 1.0);
        UV.xyz /= UV.w;
        UV.xyz = UV.xyz * 0.5 + 0.5;
        if (UV.x < 0 || UV.x > 1 || UV.y < 0 || UV.y > 1) {
            break;
        }
        float worldDepth = linearizeDepth(texture(DepthTex, UV.st).r);
        float curDepth = linearizeDepth(UV.z);
        if (abs(worldDepth) - abs(curDepth) < 0 || i == steps - 1) {
            return UV.st;
        }
    }
    return vec2(0.0);
}

void main()
{
    vec3 FragPos = texture(Position, TexCoords).rgb;
    // DiffuseColor
    vec3 Diffuse = texture(DiffuseSpecular, TexCoords).rgb;
    // feature map
    vec4 water = texture(WaterTex, TexCoords);
    // specular
    float Specular = texture(DiffuseSpecular, TexCoords).a;
    // world normal map
    vec3 normal = texture(Normal, TexCoords).rgb;          //[0,1]
    normal = normalize(2.0 * normal - 1.0);                 //[-1,1]
    // depth
    float depth = texture(DepthTex, TexCoords).r;
    vec3 waterColor = vec3(0.0);
    if (Diffuse == vec3(0.0)) {
        vec4 positionInView = view * vec4(FragPos, 1.0);
//        vec3 vDir = normalize(-positionInView.xyz);
        vec3 vDir = viewPos - FragPos;
        vDir = normalize(view * vec4(vDir, 1.0f)).xyz;
        vec3 norm = mat3(view) * normal;
        vec3 reflectDir = normalize(reflect(vDir, norm));
        vec2 uv = rayMarch(positionInView.xyz, reflectDir);
        waterColor = textureLod(DiffuseSpecular, uv, 1).rgb;
        if (waterColor != vec3(0.0)) {
//            FragColor = vec4(mix(water.rgb, waterColor.rgb, 0.5), 1.0);
            FragColor = 0.5 * vec4(waterColor.rgb, 1.0);
        } else {
            FragColor = vec4(water.rgb, 1.0);
        }
        return;
    }
    FragColor = vec4(Diffuse, 1.0);
}