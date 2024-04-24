#include "framework/global_env.hpp"

#include "model/water.hpp"
#include "model/triangle.hpp"

static std::shared_ptr<Global> instance = nullptr;
static std::once_flag singletonFlag;

Global::Global() {
    InitEnv();
}

Global::~Global() {
}

void Global::InitEnv() {
    nlohmann::json envJson = JsonConfigLoader::GetJsonObject("env/env_path.json");

    // 注册路径
    for (const auto& keyValue : envJson.items()) {
        const std::string& key = keyValue.key();
        const nlohmann::json& value = keyValue.value();
        this->envPaths[key] = value;
    }
}

std::shared_ptr<Global> Global::GetInstance() {
    std::call_once(singletonFlag, [&] {
        instance = std::make_shared<Global>();
    });
    return instance;
}

void Global::RegisterEnvPath(std::string name, std::string path) {
    this->envPaths[name] = path;
}

std::string Global::GetEnvPath(std::string name) {
    return this->envPaths[name];
}

void Global::InitAssets() {
    glEnable(GL_DEPTH_TEST);
    auto assetsMgr = this->GetMgr<AssetsMgr>();
    // 加载着色器
    assetsMgr->LoadShader("skybox", "assets/shaders/sky.vsh", "assets/shaders/sky.fsh");
    assetsMgr->LoadShader("debugger", "assets/shaders/debug.vsh", "assets/shaders/debug.fsh");
    assetsMgr->LoadShader("shadow", "assets/shaders/shadow.vsh", "assets/shaders/shadow.fsh");
    assetsMgr->LoadShader("g_model", "assets/shaders/gbuffers_model.vsh", "assets/shaders/gbuffers_model.fsh");
    assetsMgr->LoadShader("g_robot", "assets/shaders/gbuffers_robot.vsh", "assets/shaders/gbuffers_robot.fsh");
    assetsMgr->LoadShader("g_sun", "assets/shaders/gbuffers_sun.vsh", "assets/shaders/gbuffers_sun.fsh");
    assetsMgr->LoadShader("g_terrain", "assets/shaders/gbuffers_terrain.vsh", "assets/shaders/gbuffers_terrain.fsh");
    assetsMgr->LoadShader("g_water", "assets/shaders/gbuffers_water.vsh", "assets/shaders/gbuffers_water.fsh");
    assetsMgr->LoadShader("g_water_r", "assets/shaders/gbuffers_water_reflection.vsh", "assets/shaders/gbuffers_water_reflection.fsh");
    assetsMgr->LoadShader("composite1", "assets/shaders/composite1.vsh", "assets/shaders/composite1.fsh");
    assetsMgr->LoadShader("composite2", "assets/shaders/composite2.vsh", "assets/shaders/composite2.fsh");
    //    this->GetMgr<AssetsMgr>()->LoadShader("final", "assets/shaders/final.vsh", "assets/shaders/final.fsh");
    assetsMgr->LoadShader("terrain_compute", "assets/shaders/terrain_compute.comp");
    // 加载噪声图
    assetsMgr->LoadTexture("perlin_noise", "assets/textures/perlinNoise.png");
    // 加载模型
    assetsMgr->LoadModel<Cube>().Load("skybox", "assets/model/json/cube.json");
    assetsMgr->LoadModel<AssimpModel>().Load("nanosuit", "assets/model/nanosuit/nanosuit.obj");
    assetsMgr->LoadModel<Terrain>().Load("terrain", "assets/model/json/terrain.json");
    assetsMgr->LoadModel<Water>().Load("water", "assets/model/json/water.json");
    assetsMgr->LoadModel<Triangle>().Load("triangle", "assets/model/json/triangle.json");
}
