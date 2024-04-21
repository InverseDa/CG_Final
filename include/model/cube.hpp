#pragma once
#include "opengl_ext/model.hpp"
#include "framework/engine.hpp"
#include "opengl_ext/texture.hpp"

class Cube : public Model {
  private:
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;

    void VerticesSetup(const std::string& jsonPath);

  public:
    Cube() = default;
    Cube(const std::string& jsonPath);
    ~Cube() = default;

    void Draw(Shader& shader);
};