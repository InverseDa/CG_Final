#include "model/cube.hpp"

Cube::Cube(const std::string& jsonPath) : Model() {
    std::vector<std::string> facesPath{"assets/textures/skybox/right.jpg",
                                       "assets/textures/skybox/left.jpg",
                                       "assets/textures/skybox/top.jpg",
                                       "assets/textures/skybox/bottom.jpg",
                                       "assets/textures/skybox/front.jpg",
                                       "assets/textures/skybox/back.jpg"};
    this->LoadSkyBox(jsonPath, facesPath);
}

void Cube::LoadSkyBox(const std::string& jsonPath, std::vector<std::string>& faces_path) {
    Texture skyboxTex;
    skyboxTex.LoadSkyBoxTexture(faces_path);
    this->textures.push_back(skyboxTex);

    this->VerticesSetup(jsonPath);
}

void Cube::VerticesSetup(const std::string& jsonPath) {
    nlohmann::json json = Global::GetJsonObject(jsonPath);
    std::vector<std::vector<float>> vertices = json["vertices"];
    std::vector<Vertex> v;
    for (const auto& vertex : vertices) {
        auto& [x, y, z] = std::tie(vertex[0], vertex[1], vertex[2]);
        v.push_back({glm::vec3(x, y, z)});
    }
    this->meshes.emplace_back(v, this->textures, true);
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