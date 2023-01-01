/*
        ___                              ____        _        ____ ____
       |_ _|_ ____   _____ _ __ ___  ___|  _ \  __ _( )___   / ___/ ___|
        | || '_ \ \ / / _ \ '__/ __|/ _ \ | | |/ _` |// __| | |  | |  _
        | || | | \ V /  __/ |  \__ \  __/ |_| | (_| | \__ \ | |__| |_| |
       |___|_| |_|\_/ \___|_|  |___/\___|____/ \__,_| |___/  \____\____|

                    ____  ____   ___      _ _____ ____ _____
                   |  _ \|  _ \ / _ \    | | ____/ ___|_   _|
                   | |_) | |_) | | | |_  | |  _|| |     | |
                   |  __/|  _ <| |_| | |_| | |__| |___  | |
                   |_|   |_| \_\\___/ \___/|_____\____| |_|
*/

#include <iostream>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "GLM.h"
#include "Shader/Shader.h"
#include "Camera/Camera.h"
#include "stb/stb_image.h"
#include "Texture/Texture.h"
#include "Model/Model.h"
#include "GBUFFER/gbuffer.h"
#include "Robot/Robot.h"

#define White    glm::vec3(1.0, 1.0, 1.0)
#define Yellow    glm::vec3(1.0, 1.0, 0.0)
#define Green    glm::vec3(0.0, 1.0, 0.0)
#define Cyan    glm::vec3(0.0, 1.0, 1.0)
#define Magenta    glm::vec3(1.0, 0.0, 1.0)
#define Red        glm::vec3(1.0, 0.0, 0.0)
#define Black    glm::vec3(0.0, 0.0, 0.0)
#define Blue    glm::vec3(0.0, 0.0, 1.0)
#define Brown    glm::vec3(0.5, 0.5, 0.5)

const int WATER_VERTICES_HEIGHT_AND_WIDTH = 410;
const unsigned int WINDOW_WIDTH = 1366, WINDOW_HEIGHT = 768;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
const float near_plane = 0.1f, far_plane = 100000.0f;
const int RENDER_SHADOW = 0, RENDER_GBUFFER = 1, RENDER_WATER_VP = 2;

/////////////////////////////////////////////Camera/////////////////////////////////////////////
//  FPS风格摄像机
Camera camera(glm::vec3(-20.0f, 60.0f, -20.0f),
              glm::vec3(0.0f, 0.0f, 1.0f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              0.0f, -90.0f, 45.0f);
//  timing初始化
float deltaTime = 0.0f;     //  time between current frame and last frame
float lastFrame = 0.0f;
//  鼠标光标初始化
float lastX = 400;
float lastY = 300;
//  第一次使用鼠标
bool firstMouse = true;
//  太阳旋转
bool isRotate = false;
//  view矩阵
glm::mat4 view = glm::mat4(1.0f);
//投影矩阵，实质为透视矩阵
glm::mat4 projection = glm::mat4(1.0f);
GLFWwindow *window;
//lightPos
glm::vec3 lightPos(3024.0f, 1000.0f, 3024.0f);
glm::vec3 lightColor(0.95, 1.0, 0.86);
glm::mat4 lightProjection, lightView;
glm::mat4 lightSpaceMatrix;
//smallLight
glm::vec3 smallLight(800, 100, 500);
//dotCamera
glm::vec3 dotCameraPos(-20.0f, 250.0f, -20.0f);
/////////////////////////////////////////////HeightMap/////////////////////////////////////////////
//  terrain
std::vector<float> terrainVertices;
std::vector<float> terrainColor, terrainTexCoord;
std::vector<unsigned int> terrainIndices;
//  skyBox
//  water
std::vector<float> waterVertices;
std::vector<unsigned int> waterIndices;
//  heightmap arguments
int width, height, nChannels;
/////////////////////////////////////////////VAO、VBO、EBO/////////////////////////////////////////////
//  VAO VBO EBO
unsigned int terrainVAO, terrainVerticesVBO, terrainTextureVBO, terrainTexCoordVBO, terrainEBO,
        skyBoxVAO, skyBoxVBO,
        waterVAO, waterVerticesVBO, waterNormalVBO, waterVerticesEBO,
        quadVAO, quadVBO;
//SHADOW
GLuint shadowMapFBO, shadowMap,
        mirrorFBO, mirrorTex, mirrorDBO[1], mirrorRBO;
/////////////////////////////////////////////Shaders/////////////////////////////////////////////
Shader *terrainShader, *skyBoxShader, *waterShader,
        *modelShader, *shadowShader, *debugShadowShader, *sunShader,
        *composite1Shader, *composite2Shader, *waterReflectionShader,
        *robotShader;
/////////////////////////////////////////////TextureID/////////////////////////////////////////////
unsigned int skyBoxTextureID, noisetex, terrainTextureID, terrainSpecular, terrainNormal, waterTextureID;
/////////////////////////////////////////////Model/////////////////////////////////////////////
Model tree, sun, nanosuit;

gBuffer *gbuffer;


struct screen {
    void Draw(Shader shader) {
        shader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
} screen;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = (float) 200 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos += cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -= cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        lightPos.y += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        lightPos.y -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos.x += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos.x -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos.z += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos.z -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        smallLight.x += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        smallLight.x -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        smallLight.z += 5.0f;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        smallLight.z -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        smallLight.y -= 5.0f;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        isRotate = true;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isRotate = false;
}

void mouse_callback(GLFWwindow *window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    //  注意方向问题
    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    //  灵敏度
    float sensitive = 0.15f;
    xOffset *= sensitive;
    yOffset *= sensitive;

    camera.processMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
    camera.processScrollMovement(yOffset);
}

//  利用高度图生成地形terrain
void loadHeightMap() {
    int dwidth, dheight, dn;
    unsigned char *heightMap = stbi_load("textures/DefaultTerrain/Height Map.png",
                                         &width, &height, &nChannels,
                                         0);
    unsigned char *diffuseMap = stbi_load("textures/DefaultTerrain/Diffuse.png",
                                          &dwidth, &dheight, &dn,
                                          0);

    terrainTextureID = loadTexture("textures/DefaultTerrain/Diffuse.png");
    terrainSpecular = loadTexture("textures/DefaultTerrain/Height Map_SPEC.png");
    terrainNormal = loadTexture("textures/DefaultTerrain/Height Map_NORM.png");
    if (!heightMap || !diffuseMap) {
        std::cout << "Error: Load HeightMap Failed! \n";
        exit(0);
    }
    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned char *texel = heightMap + (i * width + j) * nChannels;
            unsigned char *color = diffuseMap + (i * width + j) * dn;

            unsigned char y = texel[0];
            float xx = -height / 2.0f + i,
                    yy = (int) y * yScale - yShift,
                    zz = -width / 2.0f + j;
            terrainVertices.push_back(xx);
            terrainVertices.push_back(yy);
            terrainVertices.push_back(zz);
            //Texcoords
            terrainVertices.push_back((j) * 1.0f / (float) (width - 1));
            terrainVertices.push_back((i) * 1.0f / (float) (height - 1));
            //Color（弃用）
            terrainColor.push_back((float) color[0] / 255.0f);
            terrainColor.push_back((float) color[1] / 255.0f);
            terrainColor.push_back((float) color[2] / 255.0f);
        }
    }

    stbi_image_free(heightMap);
    stbi_image_free(diffuseMap);

    //  indices计算
    for (unsigned int i = 0; i < width - 1; i++) {
        for (unsigned int j = 0; j < width; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                terrainIndices.push_back((i + k) * width + j);
            }
        }
    }
}

void loadWater() {
    const float planeHeight = 0.0f;

    //  细分水平面，切割
    std::pair<int, int> hashMap1[] = {{0, 0},
                                      {1, 0}};
    std::pair<int, int> hashMap2[] = {{0, 1},
                                      {1, 1}};
    int k = 0, index;
    for (int i = 0; i < WATER_VERTICES_HEIGHT_AND_WIDTH; i++) {
        for (int j = 0; j < WATER_VERTICES_HEIGHT_AND_WIDTH; j++, k++) {
            waterVertices.push_back(static_cast<float>(i));
            waterVertices.push_back(planeHeight);
            waterVertices.push_back(static_cast<float>(j));
            if (i % 2 == 0) {
                waterVertices.push_back(static_cast<float>(hashMap1[k % 2].first));
                waterVertices.push_back(static_cast<float>(hashMap1[k % 2].second));
            } else {
                waterVertices.push_back(static_cast<float>(hashMap2[k % 2].first));
                waterVertices.push_back(static_cast<float>(hashMap2[k % 2].second));
            }
        }
    }
    for (int i = 0; i < WATER_VERTICES_HEIGHT_AND_WIDTH - 1; i++) {
        for (int j = 0; j < WATER_VERTICES_HEIGHT_AND_WIDTH - 1; j++) {
            index = i * WATER_VERTICES_HEIGHT_AND_WIDTH + j;
            waterIndices.push_back(index);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH + j);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH + j);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH + j + 1);
        }
    }
}

void loadTreeModel() {
    tree.load("model/Tree/Tree.obj");
}

void loadSunModel() {
    sun.load("model/sun/13913_Sun_v2_l3.obj");
}

void loadPerlinNoise() {
    // 加载噪声图
//    int textureWidth, textureHeight, textureNChannels;
//    unsigned char *image = stbi_load("textures/perlinNoise.png", &textureWidth, &textureHeight, &textureNChannels, 0);
//    glGenTextures(1, &noisetex);
//    glBindTexture(GL_TEXTURE_2D, noisetex);
////    glGenerateMipmap(GL_TEXTURE_2D);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);   // 生成纹理
//    stbi_image_free(image);
    noisetex = loadTexture("textures/perlinNoise.png");
}

void loadNanoSuit() {
    nanosuit.load("model/nanosuit/nanosuit.obj");
}

void initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Terrain", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Error: Fail to create window! \n";
        glfwTerminate();
        exit(0);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Error: Fail to initialize GLAD! \n";
        exit(0);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void initSkyBox() {
    std::vector<std::string> faces{
            "textures/skybox/right.jpg",
            "textures/skybox/left.jpg",
            "textures/skybox/top.jpg",
            "textures/skybox/bottom.jpg",
            "textures/skybox/front.jpg",
            "textures/skybox/back.jpg"
    };
    float skyBoxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };

    glGenVertexArrays(1, &skyBoxVAO);
    glBindVertexArray(skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skyBoxVertices),
                 &skyBoxVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) (nullptr));

    skyBoxTextureID = loadSkyBox(faces);
}

void initTerrain() {
    loadHeightMap();
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 terrainVertices.size() * sizeof(float),
                 &terrainVertices[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &terrainTextureVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainTextureVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 terrainColor.size() * sizeof(float),
                 &terrainColor[0],
                 GL_STATIC_DRAW);
    GLuint terrainColorLocation = glGetAttribLocation(terrainShader->id, "color");
    glEnableVertexAttribArray(terrainColorLocation);
    glVertexAttribPointer(terrainColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) (nullptr));

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 terrainIndices.size() * sizeof(unsigned int),
                 &terrainIndices[0],
                 GL_STATIC_DRAW);
}

void initShaders() {
    terrainShader = new Shader("shaders/gbuffers_terrain.vsh", "shaders/gbuffers_terrain.fsh");
    skyBoxShader = new Shader("shaders/sky.vsh", "shaders/sky.fsh");
    waterShader = new Shader("shaders/gbuffers_water.vsh", "shaders/gbuffers_water.fsh");
    modelShader = new Shader("shaders/gbuffers_model.vsh", "shaders/gbuffers_model.fsh");
    shadowShader = new Shader("shaders/shadow.vsh", "shaders/shadow.fsh");
    debugShadowShader = new Shader("shaders/debug.vsh", "shaders/debug.fsh");
    sunShader = new Shader("shaders/gbuffers_sun.vsh", "shaders/gbuffers_sun.fsh");
    composite1Shader = new Shader("shaders/composite1.vsh", "shaders/composite1.fsh");
    waterReflectionShader = new Shader("shaders/gbuffers_water_reflection.vsh",
                                       "shaders/gbuffers_water_reflection.fsh");
    robotShader = new Shader("shaders/gbuffers_robot.vsh", "shaders/gbuffers_robot.fsh");
}

void initModel() {
    loadTreeModel();
    loadSunModel();
    loadNanoSuit();
}

void initDepthMap() {
    // shadow
    glGenFramebuffers(1, &shadowMapFBO);
    // create depth texture
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    debugShadowShader->use();
    debugShadowShader->setInt("depthMap", 0);
}

void initGbuffer() {
    gbuffer = new gBuffer;
#ifdef __APPLE__
    gbuffer->init(WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
#else
    gbuffer->init(WINDOW_WIDTH, WINDOW_HEIGHT);
#endif
}

void initMirrorBuffer() {
    glGenFramebuffers(1, &mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mirrorFBO);

    glGenTextures(1, &mirrorTex);
    glBindTexture(GL_TEXTURE_2D, mirrorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTex, 0);
    mirrorDBO[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, mirrorDBO);
    // create render buffer
    glGenRenderbuffers(1, &mirrorRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mirrorRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mirrorRBO);
    // check
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "FrameBuffer ERROR: status: " << status << std::endl;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void initScreen() {
    float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
}

void initWater() {
    loadWater();

    glGenVertexArrays(1, &waterVAO);
    glBindVertexArray(waterVAO);

    glGenBuffers(1, &waterVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 waterVertices.size() * sizeof(float),
                 &waterVertices[0],
                 GL_STATIC_DRAW);
    GLuint vertexLocation = glGetAttribLocation(waterShader->id, "aPos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (nullptr));
    GLuint texCoordLocation = glGetAttribLocation(waterShader->id, "texCoord");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(unsigned int),
                          (void *) (3 * sizeof(float)));

    glGenBuffers(1, &waterVerticesEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterVerticesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 waterIndices.size() * sizeof(unsigned int),
                 &waterIndices[0],
                 GL_STATIC_DRAW);

    waterTextureID = loadTexture("textures/water.jpg");
}

void initRobot() {
    // 设置物体的大小（初始的旋转和位移都为0）
    Torso->generateCube(Blue);
    Head->generateCube(Green);
    RightUpperArm->generateCube(Yellow);
    LeftUpperArm->generateCube(Yellow);
    RightUpperLeg->generateCube(Brown);
    LeftUpperLeg->generateCube(Brown);
    RightLowerArm->generateCube(Red);
    LeftLowerArm->generateCube(Red);
    RightLowerLeg->generateCube(Cyan);
    LeftLowerLeg->generateCube(Cyan);

    // 将物体的顶点数据传递
    bindObjectAndData(Torso, TorsoObject, *robotShader);
    bindObjectAndData(Head, HeadObject, *robotShader);
    bindObjectAndData(RightUpperArm, RightUpperArmObject, *robotShader);
    bindObjectAndData(LeftUpperArm, LeftUpperArmObject, *robotShader);
    bindObjectAndData(RightUpperLeg, RightUpperLegObject, *robotShader);
    bindObjectAndData(LeftUpperLeg, LeftUpperLegObject, *robotShader);
    bindObjectAndData(RightLowerArm, RightLowerArmObject, *robotShader);
    bindObjectAndData(LeftLowerArm, LeftLowerArmObject, *robotShader);
    bindObjectAndData(RightLowerLeg, RightLowerLegObject, *robotShader);
    bindObjectAndData(LeftLowerLeg, LeftLowerLegObject, *robotShader);
}

void displayTerrain(int is) {
    glm::mat4 model = glm::mat4(1.0f);
    if (is == RENDER_SHADOW) {
        shadowShader->use();
        shadowShader->set4Matrix("model", model);
        // draw mesh
        glBindVertexArray(terrainVAO);
        // render the mesh triangle strip by triangle strip - each row at a time
        for (unsigned int strip = 0; strip < height - 1; ++strip) {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                           (width * 2), // number of indices to render
                           GL_UNSIGNED_INT,     // index data type
                           (void *) (sizeof(unsigned int)
                                     * (width * 2)
                                     * strip)); // offset to starting index
        }
    } else if (is == RENDER_GBUFFER) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, terrainNormal);
        terrainShader->use();
        terrainShader->set4Matrix("model", model);
        terrainShader->set4Matrix("view", view);
        terrainShader->set4Matrix("projection", projection);
        terrainShader->setInt("tex", 0);
        terrainShader->setInt("specular", 1);
        terrainShader->setInt("normal", 2);
        // draw mesh
        glBindVertexArray(terrainVAO);
        // render the mesh triangle strip by triangle strip - each row at a time
        for (unsigned int strip = 0; strip < height - 1; ++strip) {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                           (width * 2), // number of indices to render
                           GL_UNSIGNED_INT,     // index data type
                           (void *) (sizeof(unsigned int)
                                     * (width * 2)
                                     * strip)); // offset to starting index
        }
    } else if (is == RENDER_WATER_VP) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, terrainNormal);
        waterReflectionShader->use();
        waterReflectionShader->set4Matrix("model", model);
        waterReflectionShader->setInt("tex", 0);
        waterReflectionShader->setInt("specular", 1);
        waterReflectionShader->setInt("normal", 2);
        // draw mesh
        glBindVertexArray(terrainVAO);
        // render the mesh triangle strip by triangle strip - each row at a time
        for (unsigned int strip = 0; strip < height - 1; ++strip) {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                           (width * 2), // number of indices to render
                           GL_UNSIGNED_INT,     // index data type
                           (void *) (sizeof(unsigned int)
                                     * (width * 2)
                                     * strip)); // offset to starting index
        }
    }
}

void displaySkyBox(int is) {
    glm::mat4 skyView = glm::mat4(glm::mat3(camera.getView()));// remove translation from the view matrix
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);
    if (is == RENDER_GBUFFER) {
        skyBoxShader->use();
        skyBoxShader->set4Matrix("view", skyView);
        skyBoxShader->set4Matrix("projection", projection);
        skyBoxShader->setInt("skyBox", 0);
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(skyBoxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
    }
}

void displayWater(int is) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-1024.0, 0.00000001f, -1024.0f));
    model = glm::scale(model, glm::vec3(5.0));
    if (is == RENDER_GBUFFER) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noisetex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, terrainTextureID);
        waterShader->use();
        waterShader->set4Matrix("model", model);
        waterShader->set4Matrix("view", view);
        waterShader->set4Matrix("projection", projection);
        waterShader->setFloat("worldTime", static_cast<float> (glfwGetTime()));
        waterShader->setInt("waterTexture", 0);
        waterShader->setInt("skyBox", 1);
        waterShader->setInt("noisetex", 2);
        waterShader->setInt("shadowMap", 3);
        waterShader->setInt("tex", 4);
        waterShader->set3Vector("lightPos", lightPos);
        waterShader->set3Vector("viewPos", camera.cameraPos);
        waterShader->setFloat("viewHeight", WINDOW_HEIGHT);
        waterShader->setFloat("viewWidth", WINDOW_WIDTH);
        glBindVertexArray(waterVAO);
        glDrawElements(GL_TRIANGLES, static_cast<int> (waterIndices.size()), GL_UNSIGNED_INT, nullptr);
    }
    glDisable(GL_BLEND);
}

void displayTree(int is) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 30.0f, 550.0f));
    model = glm::scale(model, glm::vec3(10.0f));
    if (is == RENDER_SHADOW) {
        shadowShader->use();
        shadowShader->set4Matrix("model", model);
        tree.Draw(*shadowShader);
    } else if (is == RENDER_GBUFFER) {
        modelShader->use();
        modelShader->set4Matrix("model", model);
        modelShader->set4Matrix("view", view);
        modelShader->set4Matrix("projection", projection);
        modelShader->setInt("isTree", 1);
        tree.Draw(*modelShader);
        modelShader->setInt("isTree", 0);
    }
}

void displaySun(int is) {
    if (isRotate) {
        float radius = 4000;
        float z = radius * static_cast<float>(cos(glfwGetTime() * 0.1)),
                y = radius * static_cast<float>(sin(glfwGetTime() * 0.1));
        lightPos.z = z, lightPos.y = y;
    }
    glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos);
    model = glm::scale(model, glm::vec3(0.1));
    if (is == RENDER_GBUFFER) {
        sunShader->use();
        sunShader->set4Matrix("model", model);
        sunShader->set4Matrix("view", view);
        sunShader->set4Matrix("projection", projection);
        sunShader->setFloat("viewHeight", WINDOW_HEIGHT);
        sunShader->setFloat("viewWidth", WINDOW_WIDTH);
        sun.Draw(*sunShader);
    }
}

void displayNano(int is) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 30.0f, 500.0f));
    model = glm::scale(model, glm::vec3(1.0f));
    if (is == RENDER_SHADOW) {
        shadowShader->use();
        shadowShader->set4Matrix("model", model);
        nanosuit.Draw(*shadowShader);
    } else if (is == RENDER_GBUFFER) {
        modelShader->use();
        modelShader->set4Matrix("model", model);
        modelShader->set4Matrix("view", view);
        modelShader->set4Matrix("projection", projection);
        nanosuit.Draw(*modelShader);
    }
}

void displayRobot(int is) {
    glm::mat4 robotView, robotProj;
    if (is == RENDER_GBUFFER) robotView = camera.getView(), robotProj = projection;
    else if (is == RENDER_SHADOW) robotView = lightView, robotProj = lightProjection;
    // 物体的变换矩阵
    glm::mat4 modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 50, 0));

    // 保持变换矩阵的栈
    MatrixStack mstack;

    // 躯干（这里我们希望机器人的躯干只绕Y轴旋转，所以只计算了RotateY）
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Torso]), glm::vec3(0.0, 1.0, 0.0));
    torso(modelMatrix, robotView, robotProj);

    mstack.push(modelMatrix); // 保存躯干变换矩阵
    // 头部（这里我们希望机器人的头部只绕Y轴旋转，所以只计算了RotateY）
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, robot.TORSO_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Head]), glm::vec3(0.0, 1.0, 0.0));
    head(modelMatrix, robotView, robotProj);
    modelMatrix = mstack.pop(); // 恢复躯干变换矩阵


    // =========== 左臂 ===========
    mstack.push(modelMatrix);   // 保存躯干变换矩阵
    // 左大臂（这里我们希望机器人的左大臂只绕Z轴旋转，所以只计算了RotateZ，后面同理）
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_ARM_WIDTH,
                                                        robot.TORSO_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperArm]), glm::vec3(0.0, 0.0, 1.0));
    left_upper_arm(modelMatrix, robotView, robotProj);

    // 左小臂
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_ARM_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerArm]), glm::vec3(0.0, 0.0, 1.0));
    left_lower_arm(modelMatrix, robotView, robotProj);
    modelMatrix = mstack.pop();

    // =========== 右臂 ===========
    mstack.push(modelMatrix);   // 保存躯干变换矩阵
    // 右大臂
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_ARM_WIDTH,
                                                        robot.TORSO_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_upper_arm(modelMatrix, robotView, robotProj);

    // 右小臂
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_ARM_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_lower_arm(modelMatrix, robotView, robotProj);
    modelMatrix = mstack.pop();

    // =========== 左腿 ===========
    // 左大腿
    mstack.push(modelMatrix);   // 保存躯干变换矩阵
    modelMatrix = glm::translate(modelMatrix,
                                 glm::vec3(-0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_LEG_WIDTH, 0, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperLeg]), glm::vec3(0.0, 0.0, 1.0));
    left_upper_leg(modelMatrix, robotView, robotProj);

    // 左小腿
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_LEG_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerLeg]), glm::vec3(0.0, 0.0, 1.0));
    left_lower_leg(modelMatrix, robotView, robotProj);
    modelMatrix = mstack.pop();


    // =========== 右腿 ===========

    // 右大腿
    mstack.push(modelMatrix);   // 保存躯干变换矩阵
    modelMatrix = glm::translate(modelMatrix,
                                 glm::vec3(0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_LEG_WIDTH, 0, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_upper_leg(modelMatrix, robotView, robotProj);

    // 右小腿
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_LEG_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_lower_leg(modelMatrix, robotView, robotProj);
    modelMatrix = mstack.pop();

}

void renderDepthMap() {
    glEnable(GL_DEPTH_TEST);
    lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 1.0f, 100000.0f);
//    lightProjection = glm::perspective(glm::radians(camera.fov), WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 0.1f, 100000.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f),
                            glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    shadowShader->use();
    shadowShader->set4Matrix("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    displayTerrain(RENDER_SHADOW);
    displayTree(RENDER_SHADOW);
    displayNano(RENDER_SHADOW);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void debugging() {
    debugShadowShader->use();
    debugShadowShader->setFloat("near", near_plane);
    debugShadowShader->setFloat("far", far_plane);
    debugShadowShader->setInt("depthMap", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    screen.Draw(*debugShadowShader);
}

void renderGBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    displayTerrain(RENDER_GBUFFER);
    displayTree(RENDER_GBUFFER);
    displayNano(RENDER_GBUFFER);
    displaySun(RENDER_GBUFFER);
    displayWater(RENDER_GBUFFER);
    displayRobot(RENDER_GBUFFER);
    displaySkyBox(RENDER_GBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderComposite() {
    composite1Shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gDiffuseSpecular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gDepthTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, noisetex);
    composite1Shader->setInt("gPosition", 0);
    composite1Shader->setInt("gNormal", 1);
    composite1Shader->setInt("gDiffuseSpecular", 2);
    composite1Shader->setInt("gDepthTex", 3);
    composite1Shader->setInt("shadowMap", 4);
    composite1Shader->setInt("noisetex", 5);
    composite1Shader->setFloat("near", near_plane);
    composite1Shader->setFloat("far", far_plane);
    composite1Shader->set4Matrix("lightSpaceMatrix", lightSpaceMatrix);
    composite1Shader->set3Vector("lightPos", smallLight);
    composite1Shader->set3Vector("viewPos", camera.cameraPos);
    composite1Shader->set3Vector("lightColor", lightColor);
    composite1Shader->set3Vector("lightDirection", glm::vec3(0) - smallLight);
    screen.Draw(*composite1Shader);
}

void freeMemory() {
    glDeleteProgram(terrainShader->id);
    glDeleteProgram(skyBoxShader->id);
    glDeleteProgram(waterShader->id);
    glDeleteProgram(modelShader->id);
    glDeleteProgram(shadowShader->id);
    glDeleteProgram(debugShadowShader->id);
    glDeleteProgram(sunShader->id);
    glDeleteProgram(composite1Shader->id);
//    glDeleteProgram(composite2Shader->id);
    glDeleteProgram(waterReflectionShader->id);
    glDeleteProgram(robotShader->id);

    delete terrainShader;
    delete skyBoxShader;
    delete waterShader;
    delete modelShader;
    delete shadowShader;
    delete debugShadowShader;
    delete sunShader;
    delete composite1Shader;
    delete composite2Shader;
    delete waterReflectionShader;
    delete robotShader;

    delete gbuffer;
}

/////////////////////////////////////////////main/////////////////////////////////////////////
int main() {
    initGLFW();
    initShaders();
    initDepthMap();
    initGbuffer();
    initTerrain();
    loadPerlinNoise();
    initWater();
    initModel();
    initScreen();
    initRobot();
    initSkyBox();

    while (!glfwWindowShouldClose(window)) {
        glEnable(GL_DEPTH_TEST);

        auto currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
/////////////////////////previous settings/////////////////////////
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = camera.getView();
        projection = glm::perspective(glm::radians(camera.fov),
                                      WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT,
                                      near_plane,
                                      far_plane);
/////////////////////////shadowMapping/////////////////////////
        renderDepthMap();
/////////////////////////rendering/////////////////////////
#ifdef __APPLE__
        glViewport(0, 0, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
#else
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
#endif
        renderGBuffer();
        renderComposite();
/////////////////////////debugger/////////////////////////
        glDisable(GL_DEPTH_TEST);
#ifdef __APPLE__
        glViewport(0, 0, WINDOW_WIDTH * 2 / 3, WINDOW_HEIGHT * 2 / 3);
#else
        glViewport(0, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3);
#endif
        debugging();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    freeMemory();
    glfwTerminate();
}

