#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "opengl_ext/shader.hpp"
#include "opengl_ext/texture.hpp"
#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

// 统一利用indices，这样好一点
class Mesh {
  public:
    // 标识符，决定是否用EBO
    bool useEBO = true;
    // 是否是天空盒子
    bool isSkyBox = false;

    unsigned int VAO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh() = default;
    Mesh(std::vector<Vertex>& vertices,
         std::vector<Texture>& textures,
         bool isSkyBox = false);
    Mesh(std::vector<Vertex>& vertices,
         std::vector<unsigned int>& indices,
         std::vector<Texture>& textures);
    virtual ~Mesh() = default;

    virtual void Draw(Shader& shader);

  protected:
    unsigned int VBO, EBO;

    virtual void SetupMesh();
};