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
    std::vector<std::vector<float>> vertices = JsonConfigLoader::Read(jsonPath, "vertices");
    std::vector<Vertex> v;
    for (const auto& vertex : vertices) {
        auto& [x, y, z] = std::tie(vertex[0], vertex[1], vertex[2]);
        v.push_back({glm::vec3(x, y, z)});
    }
    this->meshes.emplace_back(v, this->textures);
}

void Cube::Draw(Shader& shader) {
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->textures[0].id);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(this->meshes[0].VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}