#pragma once
#include "opengl_ext/mesh.hpp"
#include "opengl_ext/shader.hpp"
#include "stb/stb_image.h"
#include <vector>

class Model {
  protected:
    /* data */
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;

  public:
    Model(/* args */) = default;
    virtual ~Model() = default;

    virtual void Draw(Shader& shader);
};
