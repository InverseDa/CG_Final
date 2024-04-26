//
// Created by miaokeda on 2023/12/6.
//

#pragma once
#include "opengl_ext/model.hpp"

class Terrain : public Model {
  private:
    int width;
    int height;
    int nChannels;
    std::vector<std::pair<glm::vec3, glm::vec2>> vertices;
    std::vector<int> indices;
    GLuint VAO, VBO, EBO;

    void LoadHeightMap(const std::string& texturePath);
    void VerticesSetup();

  public:
    Terrain() = default;
    Terrain(const std::string& texturePath);
    ~Terrain() = default;

    void Draw(Shader& shader);
};
