#pragma once
#include "mgr/mgr_base.hpp"
#include "opengl_ext/model.hpp"
#include "opengl_ext/shader.hpp"
#include "opengl_ext/texture.hpp"

#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Assets Manger
 * 负责加载资源
 * 包括：材质贴图、模型、音频、字体、着色器
 */

class AssetsMgr : public MgrBase {
  private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

  public:
    AssetsMgr();
    ~AssetsMgr();

    void LoadTexture(const std::string& name, const std::string& path, const TextureType& type = TextureType::DIFFUSE);
    std::shared_ptr<Texture> GetTexture(const std::string& name);

    void LoadModel(const std::string& name, const std::string& path);
    std::shared_ptr<Model> GetModel(const std::string& name);

    void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    void LoadShader(const std::string& name, const std::string& glslPath);
    std::shared_ptr<Shader> GetShader(const std::string& name);
};