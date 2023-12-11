#pragma once
#include "opengl_ext/model.hpp"
#include "framework/engine.hpp"
#include "opengl_ext/texture.hpp"

class Cube : public Model {
  private:
    void VerticesSetup(const std::string& jsonPath);

  public:
    Cube() = default;
    Cube(const std::string& jsonPath);
    ~Cube() = default;

    void Draw(Shader& shader);
};