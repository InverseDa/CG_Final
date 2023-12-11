//
// Created by miaokeda on 2023/12/6.
//
#pragma once
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>

class JsonConfigLoader {
private:
    std::unordered_map<std::string, nlohmann::json> cache;
public:
    static nlohmann::json GetJsonObject(const std::string& jsonPath);
    static nlohmann::json Read(const std::string& path, const std::string& key);
};