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

uniform int SCR_WIDTH;
uniform int SCR_HEIGHT;

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

// reconstruct world position from depth texture
vec3 worldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverseP * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverseV * viewSpacePosition;

    return worldSpacePosition.xyz;
}

// reconstruct view position from depth texture
vec3 positionViewFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverseP * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

bool rayIsOutofScreen(vec2 ray) {
    return (ray.x > 1 || ray.y > 1 || ray.x < 0 || ray.y < 0) ? true : false;
}

vec3 rayTrace(vec3 rayPos, vec3 dir, int iterationCount) {
    float sampleDepth;
    vec3 hitColor = vec3(0.0f);
    bool hit = false;

    for(int i = 0; i < 256; i++) {
        rayPos += dir;
        if(rayIsOutofScreen(rayPos.xy)) {
            break;
        }
        sampleDepth = texture(DepthTex, rayPos.xy).r;
        float depthDif = rayPos.z - sampleDepth;
        if(depthDif >= 0 && depthDif < 0.00001) {
            hit = true;
            hitColor = texture(DiffuseSpecular, rayPos.xy).rgb;
            break;
        }
    }
    return hitColor;
}

void main()
{
    // depth
    float depth = texture(DepthTex, TexCoords).r;
    // DiffuseColor
    vec3 Diffuse = texture(DiffuseSpecular, TexCoords).rgb;
    // feature map
    vec4 water = texture(WaterTex, TexCoords);
    // specular
    float Specular = texture(DiffuseSpecular, TexCoords).a;
    // world normal map
    vec3 normal = texture(Normal, TexCoords).rgb;          //[0,1]
    normal = normalize(2.0 * normal - 1.0);                 //[-1,1]

    vec3 waterColor = vec3(0.0);
    if (Diffuse == vec3(0.0)) {
        float maxRayDistance = 100.0f;
        float wave = getWave(worldPosFromDepth(depth));

        // View Space ray calculation
        vec3 pixelPositionTexture;
        pixelPositionTexture.xy = vec2(gl_FragCoord.x / SCR_WIDTH,
                                       gl_FragCoord.y / SCR_HEIGHT);
        vec3 normalView = texture(Normal, pixelPositionTexture.xy).rgb;
        // 增加法线扰动
        normalView.z += 0.05 * (((wave - 0.4) / 0.6) * 2 - 1);
        // trans to View space
        normalView = mat3(view) * normalView;
        float pixelDepth = texture(DepthTex, pixelPositionTexture.xy).r;
        pixelPositionTexture.z = pixelDepth;
        vec4 positionView = inverseP * vec4(pixelPositionTexture * 2 - 1, 1);
        positionView /= positionView.w;
        vec3 reflectionView = normalize(reflect(positionView.xyz, normalView));
        if(reflectionView.z > 0) {
            FragColor = water;
            return;
        }
        vec3 rayEndPositionView = positionView.xyz + reflectionView * maxRayDistance;

        // UV Space ray calculation
        vec4 rayEndPositionTexture = projection * vec4(rayEndPositionView, 1);
        rayEndPositionTexture /= rayEndPositionTexture.w;
        rayEndPositionTexture.xyz = rayEndPositionTexture.xyz * 0.5 + 0.5;
        vec3 rayDirectionTexture = rayEndPositionTexture.xyz - pixelPositionTexture;

        ivec2 screenSpaceStartPosition = ivec2(pixelPositionTexture.x * SCR_WIDTH,
                                               pixelPositionTexture.y * SCR_WIDTH);
        ivec2 screenSpaceEndPosition = ivec2(rayEndPositionTexture.x * SCR_WIDTH,
                                             rayEndPositionTexture.y * SCR_WIDTH);
        ivec2 screenSpaceDistance = screenSpaceEndPosition - screenSpaceStartPosition;
        int screenSpaceMaxDistance = max(abs(screenSpaceDistance.x), abs(screenSpaceDistance.y)) / 2;
        rayDirectionTexture /= max(screenSpaceMaxDistance, 0.001f);
                                               
        // tracing
        vec3 outColor = rayTrace(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
        if(outColor == vec3(0.0f)) FragColor = water;
        else {
            outColor = outColor;
            FragColor = vec4(outColor, 1.0);
        }
        return;
    }

    FragColor = vec4(Diffuse, 1.0);
}