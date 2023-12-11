#pragma once

#include "opengl_ext/model.hpp"
#include "framework/global_env.hpp"
#include "framework/config_loader.hpp"

class Water : public Model {
  private:
    int width;
    int height;
    void VerticesSetup(const std::string& texturePath);

  public:
    Water() = default;
    explicit Water(const std::string& jsonPath);
    ~Water() override = default;

    void Draw(Shader& shader);
};
