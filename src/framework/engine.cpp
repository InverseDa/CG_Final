#include "framework/engine.hpp"
#include "mgr/assets_mgr/assets_mgr.hpp"
#include "model/cube.hpp"
#include <memory>
#include <mgr/render_mgr/render_mgr.hpp>

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
    int width = JsonConfigLoader::Read("env/settings.json", "width");
    int height = JsonConfigLoader::Read("env/settings.json", "height");
    std::string title = JsonConfigLoader::Read("env/settings.json", "title");
    this->window = WindowWrapper::createWindow(width, height, std::move(title));
}

void Engine::Update() {
    // 处理键盘和鼠标的输入
}

void Engine::Render() {
    Global::GetInstance()->GetMgr<RenderMgr>()->Run();
}

void Engine::Run() {
    while (!glfwWindowShouldClose(window->get())) {
        this->Update();
        this->Render();
    }
}