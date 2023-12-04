#pragma once
#include "opengl_ext/window.hpp"

#include <memory>

class Engine {
  private:
    std::shared_ptr<WindowWrapper*> window;
    void Init();
    void Update();
    void Render();

  public:
    Engine();
    ~Engine();

    void Run();
};
