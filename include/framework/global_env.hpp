#pragma once
#include "mgr/mgr_base.hpp"
#include "nlohmann/json.hpp"
#include "mgr/assets_mgr/assets_mgr.hpp"
#include "model/cube.hpp"
#include "model/terrain.hpp"

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
    Global();
    ~Global();
    static std::shared_ptr<Global> GetInstance();

    void InitEnv();
    void RegisterEnvPath(std::string name, std::string path);
    std::string GetEnvPath(std::string name);
    static nlohmann::json GetJsonObject(const std::string& jsonPath);

    void InitAssets();

    template <typename T>
    std::shared_ptr<T> GetMgr() {
        return T::GetInstance();
    }
};
