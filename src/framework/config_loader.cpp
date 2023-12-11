#include "framework/config_loader.hpp"

nlohmann::json JsonConfigLoader::Read(const std::string& path, const std::string& key) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file.");
    }
    nlohmann::json jsonConfig;
    file >> jsonConfig;
    file.close();
    nlohmann::json value = jsonConfig[key];

    return value;
}


nlohmann::json JsonConfigLoader::GetJsonObject(const std::string& jsonPath){
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open cube.json");
    }
    nlohmann::json json;
    ifs >> json;
    return json;
}