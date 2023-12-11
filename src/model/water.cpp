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
    std::vector<Vertex> waterVertices;
    std::vector<unsigned int> waterIndices;
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
            Vertex v;
            v.Position = position;
            v.TexCoords = texcoords;
            waterVertices.push_back(v);
        }
    }
    for (int i = 0; i < this->width - 1; i++) {
        for (int j = 0; j < this->height - 1; j++) {
            index = i * this->width + j;
            waterIndices.push_back(index);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * this->width + j);
            waterIndices.push_back(index + 1);
            waterIndices.push_back((i + 1) * this->width + j);
            waterIndices.push_back((i + 1) * this->width + j + 1);
        }
    }
    this->meshes.emplace_back(waterVertices, waterIndices, this->textures);
}

void Water::Draw(Shader& shader) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-1024.0, 0.00000001f, -1024.0f));
    model = glm::scale(model, glm::vec3(5.0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textures[0].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("skybox")->id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("perlin_noise")->id);
    glActiveTexture(GL_TEXTURE3);
    // glBindTexture(GL_TEXTURE_2D, shadowMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("terrain_diffuse")->id);
    shader.use();
    shader.setMatrix4("model", model);
    // shader.setMatrix4("view", view);
    // shader.setMatrix4("projection", projection);
    shader.setFloat("worldTime", static_cast<float>(glfwGetTime()));
    shader.setInt("waterTexture", 0);
    shader.setInt("skyBox", 1);
    shader.setInt("noisetex", 2);
    shader.setInt("shadowMap", 3);
    shader.setInt("tex", 4);
    // shader.setVector3("lightPos", lightPos);
    // shader.setVector3("viewPos", camera.cameraPos);
    // shader.setFloat("viewHeight", WINDOW_HEIGHT);
    // shader.setFloat("viewWidth", WINDOW_WIDTH);
    glBindVertexArray(this->meshes[0].VAO);
    glDrawElements(GL_TRIANGLES, static_cast<int>(this->meshes[0].indices.size()), GL_UNSIGNED_INT, nullptr);
}