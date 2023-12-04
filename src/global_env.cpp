#include "global_env.hpp"

Global::Global() {
}

Global::~Global() {
}

std::shared_ptr<Global> Global::getInstance() {
    static std::shared_ptr<Global> instance;
    if (instance == nullptr) {
        instance = std::make_shared<Global>();
    }
    return instance;
}

void Global::RegisterMgr(std::string name, std::shared_ptr<MgrBase> mgr) {
    this->mgrs[name] = mgr;
}

std::shared_ptr<MgrBase> Global::GetMgr(std::string name) {
    return this->mgrs[name];
}

void Global::RegisterEnvPath(std::string name, std::string path) {
    this->envPaths[name] = path;
}

std::string Global::GetEnvPath(std::string name) {
    return this->envPaths[name];
}
