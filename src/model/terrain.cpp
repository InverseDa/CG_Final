//
// Created by miaokeda on 2023/12/6.
//

#include "model/terrain.hpp"

Terrain::Terrain(const std::string& texturePath) : Model() {
    this->LoadHeightMap(texturePath);
}

void Terrain::LoadHeightMap(const std::string& texturePath) {
    int dwidth, dheight, dn;
    unsigned char* heightMap = stbi_load("assets/textures/DefaultTerrain/Height Map.png", &this->width, &this->height, &this->nChannels, 0);
    unsigned char* diffuseMap = stbi_load("assets/textures/DefaultTerrain/Diffuse.png", &dwidth, &dheight, &dn, 0);
    if (!heightMap || !diffuseMap) {
        std::cout << "Error: Load HeightMap Failed! \n";
        exit(0);
    }

    Texture diffuse("assets/textures/DefaultTerrain/Diffuse.png", TextureType::DIFFUSE);
    Texture specular("assets/textures/DefaultTerrain/Height Map_SPEC.png", TextureType::SPECULAR);
    Texture normal("assets/textures/DefaultTerrain/Height Map_NORM.png", TextureType::NORMAL);

    this->textures.push_back(diffuse);
    this->textures.push_back(specular);
    this->textures.push_back(normal);

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

