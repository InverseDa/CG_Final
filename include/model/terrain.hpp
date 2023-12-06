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
    void LoadHeightMap(const std::string& texturePath);
    void VerticesSetup(const std::string& texturePath);

  public:
    Terrain() = default;
    Terrain(const std::string& texturePath);
    ~Terrain() = default;

    void Draw(Shader& shader);
};
