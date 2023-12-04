#pragma once
#include "mgr/mgr_base.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class Global {
  private:
    // 动态维护管理器列表
    std::unordered_map<std::string, std::shared_ptr<MgrBase>> mgrs;
    // 路径列表
    std::unordered_map<std::string, std::string> envPaths;
    // 互斥锁
    std::mutex mtx;

  public:
    Global();
    ~Global();

    // 全局单例
    static std::shared_ptr<Global> getInstance();
    // 注册管理器
    void RegisterMgr(std::string name, std::shared_ptr<MgrBase> mgr);
    // 获取管理器
    std::shared_ptr<MgrBase> GetMgr(std::string name);
    // 注册路径
    void RegisterEnvPath(std::string name, std::string path);
    // 获取路径
    std::string GetEnvPath(std::string name);
};
