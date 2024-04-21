#pragma once
#include "mgr/mgr_base.hpp"
#include "nlohmann/json.hpp"
#include "mgr/assets_mgr/assets_mgr.hpp"
#include "model/cube.hpp"
#include "model/terrain.hpp"
#include "framework/config_loader.hpp"
#include "opengl_ext/window.hpp"

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

class Global {
  private:
    // 路径列表
    std::unordered_map<std::string, std::string> envPaths;

  public:
    // 公共全局变量
    std::shared_ptr<WindowWrapper> window;
    //// 全局光照
    glm::vec3 lightPos = glm::vec3(3024.0f, 1000.0f, 3024.0f);
    glm::vec3 lightColor = glm::vec3(0.95f, 1.0f, 0.86f);
    glm::mat4 lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 1.0f, 100000.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 1000.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 lightSpaceMatrix;
    //// 小灯光
    glm::vec3 smallLight = glm::vec3{800.f, 100.f, 500.f};

    Global();
    ~Global();
    static std::shared_ptr<Global> GetInstance();

    void InitEnv();
    void RegisterEnvPath(std::string name, std::string path);
    std::string GetEnvPath(std::string name);

    void InitAssets();

    template <typename T>
    std::shared_ptr<T> GetMgr() {
        return T::GetInstance();
    }
};
