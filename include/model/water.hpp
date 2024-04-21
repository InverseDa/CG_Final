#pragma once

#include "opengl_ext/model.hpp"
#include "framework/global_env.hpp"
#include "framework/config_loader.hpp"

class Water : public Model {
  private:
    int width;
    int height;
    std::vector<std::pair<glm::vec3, glm::vec2>> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;

    void VerticesSetup(const std::string& texturePath);

  public:
    Water() = default;
    explicit Water(const std::string& jsonPath);
    ~Water() override = default;

    void Draw(Shader& shader);
};
