
//        ___                              ____        _        ____ ____
//       |_ _|_ ____   _____ _ __ ___  ___|  _ \  __ _( )___   / ___/ ___|
//        | || '_ \ \ / / _ \ '__/ __|/ _ \ | | |/ _` |// __| | |  | |  _
//        | || | | \ V /  __/ |  \__ \  __/ |_| | (_| | \__ \ | |__| |_| |
//       |___|_| |_|\_/ \___|_|  |___/\___|____/ \__,_| |___/  \____\____|
//
//                    ____  ____   ___      _ _____ ____ _____
//                   |  _ \|  _ \ / _ \    | | ____/ ___|_   _|
//                   | |_) | |_) | | | |_  | |  _|| |     | |
//                   |  __/|  _ <| |_| | |_| | |__| |___  | |
//                   |_|   |_| \_\\___/ \___/|_____\____| |_|

#include "Camera/Camera.h"
#include "FrameBuffer/FrameBuffer.h"
#include "GLFW/glfw3.h"
#include "GLM.h"
#include "Model/Model.h"
#include "Robot/Robot.hpp"
#include "Shader/Shader.h"
#include "Texture/Texture.h"
#include "glad/glad.h"
#include "stb/stb_image.h"

#include <iostream>
#include <vector>

#define White   glm::vec3(1.0, 1.0, 1.0)
#define Yellow  glm::vec3(1.0, 1.0, 0.0)
#define Green   glm::vec3(0.0, 1.0, 0.0)
#define Cyan    glm::vec3(0.0, 1.0, 1.0)
#define Magenta glm::vec3(1.0, 0.0, 1.0)
#define Red     glm::vec3(1.0, 0.0, 0.0)
#define Black   glm::vec3(0.0, 0.0, 0.0)
#define Blue    glm::vec3(0.0, 0.0, 1.0)
#define Brown   glm::vec3(0.5, 0.5, 0.5)

const int WATER_VERTICES_HEIGHT_AND_WIDTH = 410;
const unsigned int WINDOW_WIDTH = 1366;
const unsigned int WINDOW_HEIGHT = 768;
const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;
const float near_plane = 0.1f;
const float far_plane = 100000.0f;
const int RENDER_SHADOW = 0;
const int RENDER_GBUFFER = 1;
const int RENDER_WATER_VP = 2;

// =========================================== Camera ===========================================
//  FPS style camera
Camera camera(glm::vec3(-20.0f, 60.0f, -20.0f),
              glm::vec3(0.0f, 0.0f, 1.0f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              0.0f,
              -90.0f,
              45.0f);
//  timing initialize
float deltaTime = 0.0f; //  time between current frame and last frame
float lastFrame = 0.0f;
//  mouse cursor initialize
float lastX = 400;
float lastY = 300;

bool firstMouse = true;
//  sun rotate
bool isRotate = false;
//  view matrix
glm::mat4 view = glm::mat4(1.0f);
//  projection matrix
glm::mat4 projection = glm::mat4(1.0f);
GLFWwindow* window;
// lightPosition
glm::vec3 lightPos(3024.0f, 1000.0f, 3024.0f);
glm::vec3 lightColor(0.95, 1.0, 0.86);
glm::mat4 lightProjection, lightView;
glm::mat4 lightSpaceMatrix;
// smallLight
glm::vec3 smallLight(800, 100, 500);
// ========================================= HeightMap =========================================
//  terrain
std::vector<float> terrainVertices;
std::vector<float> terrainColor, terrainTexCoord;
std::vector<unsigned int> terrainIndices;
//  skyBox
//  water
std::vector<float> waterVertices;
std::vector<unsigned int> waterIndices;
//  heightmap arguments
int width;
int height;
int nChannels;
// dynamicCubeMap
unsigned int dynamicCubeMapTexture;
unsigned int dynamicCubeMapFrameBuffers[6];
unsigned int dynamicCubeMapTexSize;
// ======================================== BufferObject ========================================
//  VAO VBO EBO
unsigned int terrainVAO;
unsigned int terrainVerticesVBO;
unsigned int terrainTextureVBO;
unsigned int terrainTexCoordVBO;
unsigned int terrainEBO;
unsigned int skyBoxVAO;
unsigned int skyBoxVBO;
unsigned int waterVAO;
unsigned int waterVerticesVBO;
unsigned int waterNormalVBO;
unsigned int waterVerticesEBO;
unsigned int quadVAO;
unsigned int quadVBO;
// SHADOW
GLuint shadowMapFBO;
GLuint shadowMap;
GLuint mirrorFBO;
GLuint mirrorTex;
GLuint mirrorDBO[1];
GLuint mirrorRBO;
// =========================================== Shader =========================================== 
Shader* terrainShader;
Shader* skyBoxShader;
Shader* waterShader;
Shader* modelShader;
Shader* shadowShader;
Shader* debugShadowShader;
Shader* sunShader;
Shader* composite1Shader;
Shader* composite2Shader;
Shader* waterReflectionShader;
Shader* robotShader;
// ========================================== TextureID ========================================== 
unsigned int skyBoxTextureID;
unsigned int noisetex;
unsigned int terrainTextureID;
unsigned int terrainSpecular;
unsigned int terrainNormal;
unsigned int waterTextureID;
// ============================================ Model ============================================ 
Model tree;
Model sun;
Model nanosuit;
// ========================================= FrameBuffer ========================================= 
FrameBuffer* gbuffer;
FrameBuffer* composite1Buffer;

/**
 * @brief screen
 * Draw: Draw an quad on the screen
 */
struct screen {
    static void Draw(Shader shader) {
        shader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
} screen;

/**
 * @brief Callback function for responding to keyboard input and output.
 *
 * @param window: glfw Window
 */
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = (float)200 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos += cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -= cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -=
            glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos +=
            glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) *
            cameraSpeed;
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

/**
 * @brief Callback function for responding to mouse input and output.
 * 
 * @param window : glfw Window
 * @param xPos : the position X of mouse
 * @param yPos : the position Y of mouse
 */
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    //  ??????????????????
    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    //  ?????????
    float sensitive = 0.15f;
    xOffset *= sensitive;
    yOffset *= sensitive;

    camera.processMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.processScrollMovement(yOffset);
}

/**
 * @brief Terrain generation using height maps.
 * 
 */
void loadHeightMap() {
    int dwidth, dheight, dn;
    unsigned char* heightMap =
        stbi_load("textures/DefaultTerrain/Height Map.png",
                  &width,
                  &height,
                  &nChannels,
                  0);
    unsigned char* diffuseMap = stbi_load(
        "textures/DefaultTerrain/Diffuse.png", &dwidth, &dheight, &dn, 0);

    terrainTextureID = loadTexture("textures/DefaultTerrain/Diffuse.png");
    terrainSpecular =
        loadTexture("textures/DefaultTerrain/Height Map_SPEC.png");
    terrainNormal = loadTexture("textures/DefaultTerrain/Height Map_NORM.png");
    if (!heightMap || !diffuseMap) {
        std::cout << "Error: Load HeightMap Failed! \n";
        exit(0);
    }
    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned char* texel = heightMap + (i * width + j) * nChannels;
            unsigned char* color = diffuseMap + (i * width + j) * dn;

            unsigned char y = texel[0];
            float xx = -height / 2.0f + i, yy = (int)y * yScale - yShift,
                  zz = -width / 2.0f + j;
            terrainVertices.push_back(xx);
            terrainVertices.push_back(yy);
            terrainVertices.push_back(zz);
            // Texcoords
            terrainVertices.push_back((j)*1.0f / (float)(width - 1));
            terrainVertices.push_back((i)*1.0f / (float)(height - 1));
            // Color????????????
            terrainColor.push_back((float)color[0] / 255.0f);
            terrainColor.push_back((float)color[1] / 255.0f);
            terrainColor.push_back((float)color[2] / 255.0f);
        }
    }

    stbi_image_free(heightMap);
    stbi_image_free(diffuseMap);

    //  indices??????
    for (unsigned int i = 0; i < width - 1; i++) {
        for (unsigned int j = 0; j < width; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                terrainIndices.push_back((i + k) * width + j);
            }
        }
    }
}

/**
 * @brief Load water function
 * 
 */
void loadWater() {
    const float planeHeight = 0.0f;

    //  ????????????????????????
    std::pair<int, int> hashMap1[] = {{0, 0}, {1, 0}};
    std::pair<int, int> hashMap2[] = {{0, 1}, {1, 1}};
    int k = 0, index;
    for (int i = 0; i < WATER_VERTICES_HEIGHT_AND_WIDTH; i++) {
        for (int j = 0; j < WATER_VERTICES_HEIGHT_AND_WIDTH; j++, k++) {
            waterVertices.push_back(static_cast<float>(i));
            waterVertices.push_back(planeHeight);
            waterVertices.push_back(static_cast<float>(j));
            if (i % 2 == 0) {
                waterVertices.push_back(
                    static_cast<float>(hashMap1[k % 2].first));
                waterVertices.push_back(
                    static_cast<float>(hashMap1[k % 2].second));
            } else {
                waterVertices.push_back(
                    static_cast<float>(hashMap2[k % 2].first));
                waterVertices.push_back(
                    static_cast<float>(hashMap2[k % 2].second));
            }
        }
    }
    for (int i = 0; i < WATER_VERTICES_HEIGHT_AND_WIDTH - 1; i++) {
        for (int j = 0; j < WATER_VERTICES_HEIGHT_AND_WIDTH - 1; j++) {
            index = i * WATER_VERTICES_HEIGHT_AND_WIDTH + j;
            waterIndices.push_back(index);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH +
                                   j);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH +
                                   j);
            waterIndices.push_back((i + 1) * WATER_VERTICES_HEIGHT_AND_WIDTH +
                                   j + 1);
        }
    }
}

void loadTreeModel() { tree.load("model/Tree/Tree.obj"); }

void loadSunModel() { sun.load("model/sun/13913_Sun_v2_l3.obj"); }

void loadPerlinNoise() {
    // ???????????????
    //    int textureWidth, textureHeight, textureNChannels;
    //    unsigned char *image = stbi_load("textures/perlinNoise.png",
    //    &textureWidth, &textureHeight, &textureNChannels, 0); glGenTextures(1,
    //    &noisetex); glBindTexture(GL_TEXTURE_2D, noisetex);
    ////    glGenerateMipmap(GL_TEXTURE_2D);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0,
    //    GL_RGB, GL_UNSIGNED_BYTE, image);   // ????????????
    //    stbi_image_free(image);
    noisetex = loadTexture("textures/perlinNoise.png");
}

void loadNanoSuit() { nanosuit.load("model/nanosuit/nanosuit.obj"); }

void initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "Terrain", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Error: Fail to create window! \n";
        glfwTerminate();
        exit(0);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Error: Fail to initialize GLAD! \n";
        exit(0);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void initSkyBox() {
    std::vector<std::string> faces{"textures/skybox/right.jpg",
                                   "textures/skybox/left.jpg",
                                   "textures/skybox/top.jpg",
                                   "textures/skybox/bottom.jpg",
                                   "textures/skybox/front.jpg",
                                   "textures/skybox/back.jpg"};
    float skyBoxVertices[] = {
        // positions
        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    glGenVertexArrays(1, &skyBoxVAO);
    glBindVertexArray(skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skyBoxVertices),
                 &skyBoxVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(nullptr));

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
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &terrainTextureVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainTextureVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 terrainColor.size() * sizeof(float),
                 &terrainColor[0],
                 GL_STATIC_DRAW);
    GLuint terrainColorLocation =
        glGetAttribLocation(terrainShader->id, "color");
    glEnableVertexAttribArray(terrainColorLocation);
    glVertexAttribPointer(terrainColorLocation,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          3 * sizeof(float),
                          (void*)(nullptr));

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 terrainIndices.size() * sizeof(unsigned int),
                 &terrainIndices[0],
                 GL_STATIC_DRAW);
}

void initShaders() {
    terrainShader = new Shader("shaders/gbuffers_terrain.vsh",
                               "shaders/gbuffers_terrain.fsh");
    skyBoxShader = new Shader("shaders/sky.vsh", "shaders/sky.fsh");
    waterShader =
        new Shader("shaders/gbuffers_water.vsh", "shaders/gbuffers_water.fsh");
    modelShader =
        new Shader("shaders/gbuffers_model.vsh", "shaders/gbuffers_model.fsh");
    shadowShader = new Shader("shaders/shadow.vsh", "shaders/shadow.fsh");
    debugShadowShader = new Shader("shaders/debug.vsh", "shaders/debug.fsh");
    sunShader =
        new Shader("shaders/gbuffers_sun.vsh", "shaders/gbuffers_sun.fsh");
    composite1Shader =
        new Shader("shaders/composite1.vsh", "shaders/composite1.fsh");
    waterReflectionShader = new Shader("shaders/gbuffers_water_reflection.vsh",
                                       "shaders/gbuffers_water_reflection.fsh");
    robotShader =
        new Shader("shaders/gbuffers_robot.vsh", "shaders/gbuffers_robot.fsh");
    composite2Shader =
        new Shader("shaders/composite2.vsh", "shaders/composite2.fsh");
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
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initGbuffer() {
    gbuffer = new FrameBuffer;
    composite1Buffer = new FrameBuffer;
#ifdef __APPLE__
    gbuffer->init(WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
    composite1Buffer->init(WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
#else
    gbuffer->init(WINDOW_WIDTH, WINDOW_HEIGHT);
    composite1Buffer->init(WINDOW_WIDTH, WINDOW_HEIGHT);
#endif
}

void initScreen() {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
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
    glVertexAttribPointer(vertexLocation,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(float),
                          (void*)(nullptr));
    GLuint texCoordLocation = glGetAttribLocation(waterShader->id, "texCoord");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(unsigned int),
                          (void*)(3 * sizeof(float)));

    glGenBuffers(1, &waterVerticesEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterVerticesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 waterIndices.size() * sizeof(unsigned int),
                 &waterIndices[0],
                 GL_STATIC_DRAW);

    waterTextureID = loadTexture("textures/water.jpg");
}

void initRobot() {
    // ??????????????????????????????????????????????????????0???
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

    // ??????????????????????????????
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

void initDynamicCubeMap() {
    dynamicCubeMapTexSize = 1024;
    glGenTextures(1, &dynamicCubeMapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, dynamicCubeMapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB,
                     dynamicCubeMapTexSize,
                     dynamicCubeMapTexSize,
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     nullptr);
    }

    glGenFramebuffers(6, dynamicCubeMapFrameBuffers);
    for (unsigned int i = 0; i < 6; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, dynamicCubeMapFrameBuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               dynamicCubeMapTexture,
                               0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "FrameBuffer ERROR! " << std::endl;
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER,
                              GL_DEPTH24_STENCIL8,
                              dynamicCubeMapTexSize,
                              dynamicCubeMapTexSize);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
            glDrawElements(GL_TRIANGLE_STRIP, // primitive type
                           (width * 2),       // number of indices to render
                           GL_UNSIGNED_INT,   // index data type
                           (void*)(sizeof(unsigned int) * (width * 2) *
                                   strip)); // offset to starting index
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
            glDrawElements(GL_TRIANGLE_STRIP, // primitive type
                           (width * 2),       // number of indices to render
                           GL_UNSIGNED_INT,   // index data type
                           (void*)(sizeof(unsigned int) * (width * 2) *
                                   strip)); // offset to starting index
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
            glDrawElements(GL_TRIANGLE_STRIP, // primitive type
                           (width * 2),       // number of indices to render
                           GL_UNSIGNED_INT,   // index data type
                           (void*)(sizeof(unsigned int) * (width * 2) *
                                   strip)); // offset to starting index
        }
    }
}

void displaySkyBox(int is) {
    glm::mat4 skyView = glm::mat4(
        glm::mat3(camera.getView())); // remove translation from the view matrix
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
    //    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                     glm::vec3(-1024.0, 0.00000001f, -1024.0f));
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
        waterShader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
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
        glDrawElements(GL_TRIANGLES,
                       static_cast<int>(waterIndices.size()),
                       GL_UNSIGNED_INT,
                       nullptr);
    }
    //    glDisable(GL_BLEND);
}

void displayTree(int is) {
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 30.0f, 550.0f));
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
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(800.0f, 30.0f, 500.0f));
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
    bool isShadow = false;
    if (is == RENDER_SHADOW)
        isShadow = true;
    // ?????????????????????
    glm::mat4 modelMatrix = glm::mat4(1.0);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(810, 30, 500));

    // ????????????????????????
    MatrixStack mstack;

    // ???????????????????????????????????????????????????Y??????????????????????????????RotateY???
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.Torso]),
                              glm::vec3(0.0, 1.0, 0.0));
    torso(isShadow, shadowShader, modelMatrix, view, projection);

    mstack.push(modelMatrix); // ????????????????????????
    // ???????????????????????????????????????????????????Y??????????????????????????????RotateY???
    modelMatrix =
        glm::translate(modelMatrix, glm::vec3(0.0, robot.TORSO_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.Head]),
                              glm::vec3(0.0, 1.0, 0.0));
    head(isShadow, shadowShader, modelMatrix, view, projection);
    modelMatrix = mstack.pop(); // ????????????????????????

    // =========== ?????? ===========
    mstack.push(modelMatrix); // ????????????????????????
    // ?????????????????????????????????????????????????????????Z??????????????????????????????RotateZ??????????????????
    modelMatrix = glm::translate(
        modelMatrix,
        glm::vec3(-0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_ARM_WIDTH,
                  robot.TORSO_HEIGHT,
                  0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.LeftUpperArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    left_upper_arm(isShadow, shadowShader, modelMatrix, view, projection);

    // ?????????
    modelMatrix =
        glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_ARM_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.LeftLowerArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    left_lower_arm(isShadow, shadowShader, modelMatrix, view, projection);
    modelMatrix = mstack.pop();

    // =========== ?????? ===========
    mstack.push(modelMatrix); // ????????????????????????
    // ?????????
    modelMatrix = glm::translate(
        modelMatrix,
        glm::vec3(0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_ARM_WIDTH,
                  robot.TORSO_HEIGHT,
                  0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.RightUpperArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_upper_arm(isShadow, shadowShader, modelMatrix, view, projection);

    // ?????????
    modelMatrix =
        glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_ARM_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.RightLowerArm]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_lower_arm(isShadow, shadowShader, modelMatrix, view, projection);
    modelMatrix = mstack.pop();

    // =========== ?????? ===========
    // ?????????
    mstack.push(modelMatrix); // ????????????????????????
    modelMatrix = glm::translate(
        modelMatrix,
        glm::vec3(
            -0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_LEG_WIDTH, 0, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.LeftUpperLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    left_upper_leg(isShadow, shadowShader, modelMatrix, view, projection);

    // ?????????
    modelMatrix =
        glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_LEG_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.LeftLowerLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    left_lower_leg(isShadow, shadowShader, modelMatrix, view, projection);
    modelMatrix = mstack.pop();

    // =========== ?????? ===========

    // ?????????
    mstack.push(modelMatrix); // ????????????????????????
    modelMatrix = glm::translate(
        modelMatrix,
        glm::vec3(
            0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_LEG_WIDTH, 0, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.RightUpperLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_upper_leg(isShadow, shadowShader, modelMatrix, view, projection);

    // ?????????
    modelMatrix =
        glm::translate(modelMatrix, glm::vec3(0, -robot.LOWER_LEG_HEIGHT, 0.0));
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(robot.theta[robot.RightLowerLeg]),
                              glm::vec3(0.0, 0.0, 1.0));
    right_lower_leg(isShadow, shadowShader, modelMatrix, view, projection);
    modelMatrix = mstack.pop();
}

void renderDepthMap() {
    glEnable(GL_DEPTH_TEST);
    lightProjection =
        glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 1.0f, 100000.0f);
    //    lightProjection = glm::perspective(glm::radians(camera.fov),
    //    WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT, 0.1f, 100000.0f);
    lightView =
        glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
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
    displayRobot(RENDER_SHADOW);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void debugging() {
    debugShadowShader->use();
    debugShadowShader->setFloat("near", near_plane);
    debugShadowShader->setFloat("far", far_plane);
    debugShadowShader->setInt("depthMap", 0);
    debugShadowShader->setInt("gDepthTex", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->DepthTex);

    screen::Draw(*debugShadowShader);
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

void renderComposite1() {
    glBindFramebuffer(GL_FRAMEBUFFER, composite1Buffer->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    composite1Shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->Position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->Normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->DiffuseSpecular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->DepthTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gbuffer->WaterTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, noisetex);
    composite1Shader->setInt("gPosition", 0);
    composite1Shader->setInt("gNormal", 1);
    composite1Shader->setInt("gDiffuseSpecular", 2);
    composite1Shader->setInt("gDepthTex", 3);
    composite1Shader->setInt("gWaterTex", 4);
    composite1Shader->setInt("shadowMap", 5);
    composite1Shader->setInt("noisetex", 6);
    composite1Shader->setFloat("near", near_plane);
    composite1Shader->setFloat("far", far_plane);
    composite1Shader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    composite1Shader->set4Matrix("view", view);
    composite1Shader->set4Matrix("viewInverse", glm::inverse(view));
    composite1Shader->set4Matrix("projection", projection);
    composite1Shader->set4Matrix("projectionInverse", glm::inverse(projection));
    composite1Shader->set4Matrix("lightSpaceMatrix", lightSpaceMatrix);
    composite1Shader->set3Vector("lightPos", lightPos);
    composite1Shader->set3Vector("viewPos", camera.cameraPos);
    composite1Shader->set3Vector("lightColor", lightColor);
    composite1Shader->set3Vector("lightDirection", glm::vec3(0) - lightPos);
    screen::Draw(*composite1Shader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderComposite2() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    composite2Shader->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, composite1Buffer->Position);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, composite1Buffer->DiffuseSpecular);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->DepthTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, composite1Buffer->WaterTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, composite1Buffer->Normal);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, noisetex);
    composite2Shader->setInt("Position", 1);
    composite2Shader->setInt("DiffuseSpecular", 2);
    composite2Shader->setInt("DepthTex", 3);
    composite2Shader->setInt("WaterTex", 4);
    composite2Shader->setInt("Normal", 5);
    composite2Shader->setInt("noisetex", 6);
    composite2Shader->setFloat("near", near_plane);
    composite2Shader->setFloat("far", far_plane);
    composite2Shader->setFloat("worldTime", static_cast<float>(glfwGetTime()));
    composite2Shader->set4Matrix("view", view);
    composite2Shader->set4Matrix("inverseV", glm::inverse(view));
    composite2Shader->set4Matrix("projection", projection);
    composite2Shader->set4Matrix("inverseP", glm::inverse(projection));
    composite2Shader->set4Matrix("inverseVP", glm::inverse(projection * view));
    composite2Shader->set4Matrix("lightSpaceMatrix", lightSpaceMatrix);
    composite2Shader->set3Vector("lightPos", lightPos);
    composite2Shader->set3Vector("viewPos", camera.cameraPos);
    composite2Shader->set3Vector("lightColor", lightColor);
    composite2Shader->set3Vector("lightDirection", glm::vec3(0) - lightPos);
#ifdef __APPLE__
    composite2Shader->setInt("SCR_WIDTH", WINDOW_WIDTH * 2);
    composite2Shader->setInt("SCR_HEIGHT", WINDOW_HEIGHT * 2);
#else
    composite2Shader->setInt("SCR_WIDTH", WINDOW_WIDTH);
    composite2Shader->setInt("SCR_HEIGHT", WINDOW_HEIGHT);
#endif
    screen::Draw(*composite2Shader);
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
    delete composite1Buffer;
}

// ============================================ Main ============================================ 
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

        auto currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // ===================== Previous Settings ======================
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = camera.getView();
        projection = glm::perspective(glm::radians(camera.fov),
                                      WINDOW_WIDTH * 1.0f / WINDOW_HEIGHT,
                                      near_plane,
                                      far_plane);
        // ====================== Shadow Mapping =======================
        renderDepthMap();
        // ======================== Rendering ==========================
#ifdef __APPLE__
        glViewport(0, 0, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);
#else
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
#endif
        renderGBuffer();
        renderComposite1();
        renderComposite2();
        // ======================== Debugger ===========================
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
