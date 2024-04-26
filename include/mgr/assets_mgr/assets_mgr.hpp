#pragma once
#include "mgr/mgr_base.hpp"
#include "opengl_ext/assimp_model.hpp"
#include "opengl_ext/shader.hpp"
#include "opengl_ext/texture.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

/**
 * @brief Assets Manger
 * 负责加载资源
 * 包括：材质贴图、模型、音频、字体、着色器
 */

class AssetsMgr : public MgrBase {
  protected:
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    template <typename _Model>
    struct ModelStruct {
        static std::unordered_map<std::string, std::shared_ptr<_Model>>& map() {
            static std::unordered_map<std::string, std::shared_ptr<_Model>> instance;
            return instance;
        }
    };

  public:
    AssetsMgr();
    ~AssetsMgr();

    static std::shared_ptr<AssetsMgr> GetInstance();

    // Model
    template <typename _Model>
    struct ModelLoader {
        void Load(const std::string& name, const std::string& path) {
            ModelStruct<_Model>::map()[name] = std::make_shared<_Model>(path);
        }
        void Load(const std::string& name) {
            ModelStruct<_Model>::map()[name] = std::make_shared<_Model>();
        }
    };
    template <typename _Model>
    ModelLoader<_Model> LoadModel() {
        return ModelLoader<_Model>();
    }
    template <typename _Model>
    std::shared_ptr<_Model> GetModel(const std::string& name) {
        return ModelStruct<_Model>::map()[name];
    }

    // Texture
    void LoadTexture(const std::string& name, const std::string& path, const TextureType& type = TextureType::DIFFUSE);
    void LoadSkyBoxTexture(const std::string& name, std::vector<std::string>& faces);
    std::shared_ptr<Texture> GetTexture(const std::string& name);

    // Shader
    void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    void LoadShader(const std::string& name, const std::string& computeShaderPath);
    std::shared_ptr<Shader> GetShader(const std::string& name);
};
