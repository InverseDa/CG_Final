#pragma once
#include "global_env.hpp"

#include <memory>

class Engine {
  private:
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
