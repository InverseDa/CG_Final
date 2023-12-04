#include "mgr/assets_mgr/assets_mgr.hpp"

AssetsMgr::AssetsMgr() : MgrBase("AssetsMgr") {
}

AssetsMgr::~AssetsMgr() {
}

void AssetsMgr::LoadTexture(const std::string& name, const std::string& path, const TextureType& type) {
    this->textures[name] = std::make_shared<Texture>(path, type);
}

std::shared_ptr<Texture> AssetsMgr::GetTexture(const std::string& name) {
    return this->textures[name];
}

void AssetsMgr::LoadModel(const std::string& name, const std::string& path) {
    this->models[name] = std::make_shared<Model>(path);
}

std::shared_ptr<Model> AssetsMgr::GetModel(const std::string& name) {
    return this->models[name];
}

void AssetsMgr::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    this->shaders[name] = std::make_shared<Shader>(vertexPath, fragmentPath);
}

void AssetsMgr::LoadShader(const std::string& name, const std::string& glslPath) {
    this->shaders[name] = std::make_shared<Shader>(glslPath);
}

std::shared_ptr<Shader> AssetsMgr::GetShader(const std::string& name) {
    return this->shaders[name];
}