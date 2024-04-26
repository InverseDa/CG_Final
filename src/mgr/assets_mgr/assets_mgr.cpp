#include "mgr/assets_mgr/assets_mgr.hpp"

static std::shared_ptr<AssetsMgr> instance = nullptr;
static std::once_flag singletonFlag;

AssetsMgr::AssetsMgr() : MgrBase("AssetsMgr") {
}

AssetsMgr::~AssetsMgr() {
}

std::shared_ptr<AssetsMgr> AssetsMgr::GetInstance() {
    std::call_once(singletonFlag, [&] {
        instance = std::make_shared<AssetsMgr>();
    });
    return instance;
}

void AssetsMgr::LoadTexture(const std::string& name, const std::string& path, const TextureType& type) {
    this->textures[name] = std::make_shared<Texture>(path, type);
}

void AssetsMgr::LoadSkyBoxTexture(const std::string& name, std::vector<std::string>& faces) {
    this->textures[name] = std::make_shared<Texture>(faces);
}

std::shared_ptr<Texture> AssetsMgr::GetTexture(const std::string& name) {
    return this->textures[name];
}


void AssetsMgr::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    this->shaders[name] = std::make_shared<Shader>(vertexPath, fragmentPath);
}

void AssetsMgr::LoadShader(const std::string& name, const std::string& computeShaderPath) {
    this->shaders[name] = std::make_shared<Shader>(computeShaderPath);
}

std::shared_ptr<Shader> AssetsMgr::GetShader(const std::string& name) {
    return this->shaders[name];
}