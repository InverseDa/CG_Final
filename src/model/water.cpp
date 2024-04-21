#include "model/water.hpp"

Water::Water(const std::string& jsonPath) {
    std::string texturePath = JsonConfigLoader::Read(jsonPath, "diffuse_path");
    this->width = JsonConfigLoader::Read(jsonPath, "width");
    this->height = JsonConfigLoader::Read(jsonPath, "height");
    Global::GetInstance()->GetMgr<AssetsMgr>()->LoadTexture("water_diffuse", texturePath);
    this->textures.emplace_back(*Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("water_diffuse"));
    VerticesSetup(texturePath);
}

void Water::VerticesSetup(const std::string& texturePath) {
    //  细分水平面，切割
    std::pair<int, int> hashMap1[] = {{0, 0}, {1, 0}};
    std::pair<int, int> hashMap2[] = {{0, 1}, {1, 1}};
    int k = 0, index;

    for (int i = 0; i < this->width; i++) {
        for (int j = 0; j < this->height; j++, k++) {
            glm::vec3 position = glm::vec3(static_cast<float>(i), 0.0f, static_cast<float>(j));
            glm::vec2 texcoords;
            if (i % 2 == 0)
                texcoords = glm::vec2(static_cast<float>(hashMap1[k % 2].first),
                                      static_cast<float>(hashMap1[k % 2].second));
            else
                texcoords = glm::vec2(static_cast<float>(hashMap2[k % 2].first),
                                      static_cast<float>(hashMap2[k % 2].second));
            this->vertices.push_back({position, texcoords});
        }
    }
    for (int i = 0; i < this->width - 1; i++) {
        for (int j = 0; j < this->height - 1; j++) {
            index = i * this->width + j;
            this->indices.push_back(index);
            this->indices.push_back(index + 1);
            this->indices.push_back((i + 1) * this->width + j);
            this->indices.push_back(index + 1);
            this->indices.push_back((i + 1) * this->width + j);
            this->indices.push_back((i + 1) * this->width + j + 1);
        }
    }
    // vao
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    // vbo
    glGenBuffers(1, &this->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(std::pair<glm::vec3, glm::vec2>), this->vertices.data(), GL_STATIC_DRAW);
    // attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    // ebo
    glGenBuffers(1, &this->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Water::Draw(Shader& shader) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textures[0].id);
    shader.use();
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(this->indices.size()), GL_UNSIGNED_INT, nullptr);
}