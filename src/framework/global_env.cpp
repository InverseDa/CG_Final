#include "framework/global_env.hpp"

#include <model/water.hpp>

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
    // 加载着色器
    this->GetMgr<AssetsMgr>()->LoadShader("skybox", "assets/shaders/sky.vsh", "assets/shaders/sky.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("debugger", "assets/shaders/debug.vsh", "assets/shaders/debug.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("shadow", "assets/shaders/shadow.vsh", "assets/shaders/shadow.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_model", "assets/shaders/gbuffers_model.vsh", "assets/shaders/gbuffers_model.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_robot", "assets/shaders/gbuffers_robot.vsh", "assets/shaders/gbuffers_robot.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_sun", "assets/shaders/gbuffers_sun.vsh", "assets/shaders/gbuffers_sun.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_terrain", "assets/shaders/gbuffers_terrain.vsh", "assets/shaders/gbuffers_terrain.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_water", "assets/shaders/gbuffers_water.vsh", "assets/shaders/gbuffers_water.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("g_water_r", "assets/shaders/gbuffers_water_reflection.vsh", "assets/shaders/gbuffers_water_reflection.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("composite1", "assets/shaders/composite1.vsh", "assets/shaders/composite1.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("composite2", "assets/shaders/composite2.vsh", "assets/shaders/composite2.fsh");
    this->GetMgr<AssetsMgr>()->LoadShader("final", "assets/shaders/final.vsh", "assets/shaders/final.fsh");
    // 加载噪声图
    this->GetMgr<AssetsMgr>()->LoadTexture("perlin_noise", "assets/textures/perlinNoise.png");
    // 加载模型
    this->GetMgr<AssetsMgr>()->LoadModel<Cube>().Load("skybox", "assets/model/json/cube.json");
    this->GetMgr<AssetsMgr>()->LoadModel<AssimpModel>().Load("nanosuit", "assets/model/nanosuit/nanosuit.obj");
    this->GetMgr<AssetsMgr>()->LoadModel<Terrain>().Load("terrain", "assets/model/json/terrain.json");
    this->GetMgr<AssetsMgr>()->LoadModel<Water>().Load("water", "assets/model/json/water.json");
}
