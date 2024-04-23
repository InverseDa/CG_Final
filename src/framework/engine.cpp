#include "framework/engine.hpp"
#include "mgr/assets_mgr/assets_mgr.hpp"
#include "model/cube.hpp"
#include <memory>
#include <mgr/render_mgr/render_mgr.hpp>

std::chrono::time_point<std::chrono::steady_clock> currentTime{};

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
    auto ctx = Global::GetInstance();
    int width = JsonConfigLoader::Read("env/settings.json", "width");
    int height = JsonConfigLoader::Read("env/settings.json", "height");
    std::string title = JsonConfigLoader::Read("env/settings.json", "title");
    ctx->window = WindowWrapper::createWindow(width, height, std::move(title));
}

void Engine::Update() {
    // 处理键盘和鼠标的输入
    // 高精度时间计算
    auto ctx = Global::GetInstance();
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    auto cameraMgr = Global::GetInstance()->GetMgr<CameraMgr>();
    cameraMgr->ProcessKeyboardMovement(ctx->window->get(), frameTime);
    cameraMgr->ProcessMouseMovement(ctx->window->get(), frameTime);
}

void Engine::Render() {
    Global::GetInstance()->GetMgr<RenderMgr>()->Run();
}

void Engine::SetDefaultColor() {
    // 设置默认清屏颜色
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::Run() {
    auto ctx = Global::GetInstance();
    currentTime = std::chrono::high_resolution_clock::now();
    while (!ctx->window->shouldClose()) {
        // 设置默认清屏颜色
        this->SetDefaultColor();
        this->Update();
        this->Render();
        ctx->window->swapBuffers();
        ctx->window->pollEvents();
    }
}