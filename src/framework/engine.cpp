#include "framework/engine.hpp"
#include "mgr/assets_mgr/assets_mgr.hpp"
#include "model/cube.hpp"
#include <memory>

Engine::Engine() {
    this->Init();
}

Engine::~Engine() {
}

void Engine::Init() {
    // 初始化窗口
    this->InitWindow();
    // 初始化资产（加载模型、着色器、贴图）
    Global::GetInstance()->InitAssets();
}

void Engine::InitWindow() {
    // 初始化窗口
    int width = std::stoi(Global::GetInstance()->GetEnvPath("width"));
    int height = std::stoi(Global::GetInstance()->GetEnvPath("height"));
    std::string title = Global::GetInstance()->GetEnvPath("title");
    this->window = WindowWrapper::createWindow(width, height, std::move(title));
}

void Engine::Update() {
}

void Engine::Render() {
}

void Engine::Run() {
    while (true) {
        this->Update();
        this->Render();
    }
}