//
// Created by miaokeda on 2023/12/6.
//

#include "model/terrain.hpp"

#include <framework/config_loader.hpp>
#include <framework/global_env.hpp>

Terrain::Terrain(const std::string& texturePath) : Model() {
    this->LoadHeightMap(texturePath);
}

void Terrain::LoadHeightMap(const std::string& texturePath) {
    std::string heightMapPath = JsonConfigLoader::Read("assets/model/json/terrain.json", "height_path");
    std::string diffusePath = JsonConfigLoader::Read("assets/model/json/terrain.json", "diffuse_path");
    std::string specularPath = JsonConfigLoader::Read("assets/model/json/terrain.json", "specular_path");
    std::string normalPath = JsonConfigLoader::Read("assets/model/json/terrain.json", "normal_path");

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

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float yScale = 256.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < this->height; i++) {
        for (unsigned int j = 0; j < this->width; j++) {
            unsigned char* texel = heightMap + (i * this->width + j) * this->nChannels;
            unsigned char y = texel[0];
            float xx = -this->height / 2.0f + i, yy = (int)y * yScale - yShift, zz = -this->width / 2.0f + j;
            Vertex v;
            v.Position = glm::vec3(xx, yy, zz);
            v.TexCoords = glm::vec2((float)j / (float)(this->width - 1), (float)i / (float)(this->height - 1));
            vertices.emplace_back(v);
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

    this->meshes.emplace_back(vertices, indices, this->textures);
}

void Terrain::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(this->meshes[0].VAO);
    for (unsigned int strip = 0; strip < height - 1; ++strip) {
        glDrawElements(GL_TRIANGLE_STRIP, (width * 2), GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * (width * 2) * strip));
    }
}

