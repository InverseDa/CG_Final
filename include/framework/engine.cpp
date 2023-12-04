#include "framework/engine.hpp"

// Path: src/framework/engine.cpp

Engine::Engine() {
    this->Init();
}

Engine::~Engine() {
}

void Engine::Init() {
    // 初始化所有管理器

    // 初始化窗口

    // 初始化渲染器

    // 初始化场景
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