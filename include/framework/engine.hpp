#pragma once
#include "global_env.hpp"
#include "opengl_ext/window.hpp"

#include <memory>

class Engine {
  private:
    std::shared_ptr<WindowWrapper> window;
    void Init();
    void InitWindow();
    void Update();
    void Render();
    void SetDefaultColor();

  public:
    Engine();
    ~Engine();

    void Run();
};
