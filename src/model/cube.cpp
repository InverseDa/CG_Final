#include "model/cube.hpp"

Cube::Cube(const std::string& jsonPath) : Model() {
    std::vector<std::string> facesPath{"assets/textures/skybox/right.jpg",
                                       "assets/textures/skybox/left.jpg",
                                       "assets/textures/skybox/top.jpg",
                                       "assets/textures/skybox/bottom.jpg",
                                       "assets/textures/skybox/front.jpg",
                                       "assets/textures/skybox/back.jpg"};
    Global::GetInstance()->GetMgr<AssetsMgr>()->LoadSkyBoxTexture("skybox", facesPath);
    this->textures.push_back(*Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("skybox"));

    this->VerticesSetup(jsonPath);
}

void Cube::VerticesSetup(const std::string& jsonPath) {
    this->vertices = {
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f, -1.0f)
    };
   this->indices = {
        0, 1, 2,
        0, 2, 3,
        1, 5, 6,
        1, 6, 2,
        5, 4, 7,
        5, 7, 6,
        4, 0, 3,
        4, 3, 7,
        4, 5, 1,
        4, 1, 0,
        3, 2, 6,
        3, 6, 7
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), this->vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cube::Draw(Shader& shader) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0].id);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glDepthFunc(GL_LESS);
}