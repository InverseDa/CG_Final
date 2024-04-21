//
// Created by miaokeda on 2023/12/6.
//

#include "model/terrain.hpp"

#include <framework/config_loader.hpp>
#include <framework/global_env.hpp>

Terrain::Terrain(const std::string& texturePath) : Model() {
    this->LoadHeightMap(texturePath);
    this->VerticesSetup();
}

void Terrain::LoadHeightMap(const std::string& texturePath) {
    std::string heightMapPath = JsonConfigLoader::Read(texturePath, "height_path");
    std::string diffusePath = JsonConfigLoader::Read(texturePath, "diffuse_path");
    std::string specularPath = JsonConfigLoader::Read(texturePath, "specular_path");
    std::string normalPath = JsonConfigLoader::Read(texturePath, "normal_path");

    int dwidth, dheight, dn;
    unsigned char* heightMap = stbi_load(heightMapPath.c_str(), &this->width, &this->height, &this->nChannels, 0);
    unsigned char* diffuseMap = stbi_load(diffusePath.c_str(), &dwidth, &dheight, &dn, 0);
    if (!heightMap || !diffuseMap) {
        std::cout << "Error: Load HeightMap Failed! \n";
        exit(0);
    }

    Global::GetInstance()->GetMgr<AssetsMgr>()->LoadTexture("terrain_diffuse", diffusePath, TextureType::DIFFUSE);
    Global::GetInstance()->GetMgr<AssetsMgr>()->LoadTexture("terrain_specular", specularPath, TextureType::SPECULAR);
    Global::GetInstance()->GetMgr<AssetsMgr>()->LoadTexture("terrain_normal", normalPath, TextureType::NORMAL);

    this->textures.push_back(*Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("terrain_diffuse").get());
    this->textures.push_back(*Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("terrain_specular").get());
    this->textures.push_back(*Global::GetInstance()->GetMgr<AssetsMgr>()->GetTexture("terrain_normal").get());

    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < this->height; i++) {
        for (unsigned int j = 0; j < this->width; j++) {
            unsigned char* texel = heightMap + (i * this->width + j) * this->nChannels;
            unsigned char y = texel[0];
            float xx = -this->height / 2.0f + i, yy = (int)y * yScale - yShift, zz = -this->width / 2.0f + j;
            vertices.emplace_back(glm::vec3(xx, yy, zz), glm::vec2((float)j / (float)(this->width - 1), (float)i / (float)(this->height - 1)));
        }
    }

    stbi_image_free(heightMap);
    stbi_image_free(diffuseMap);

    //  indices计算
    for (unsigned int i = 0; i < this->width - 1; i++) {
        for (unsigned int j = 0; j < this->width; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                indices.push_back((i + k) * this->width + j);
            }
        }
    }
}

void Terrain::VerticesSetup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(std::pair<glm::vec3, glm::vec2>), &vertices[0], GL_STATIC_DRAW);

    // attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(std::pair<glm::vec3, glm::vec2>), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void Terrain::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textures[0].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->textures[1].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->textures[2].id);
    for (unsigned int strip = 0; strip < height - 1; ++strip) {
        glDrawElements(GL_TRIANGLE_STRIP, (width * 2), GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * (width * 2) * strip));
    }
}
