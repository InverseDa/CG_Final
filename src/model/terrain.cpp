#include "model/terrain.hpp"
#include "framework/config_loader.hpp"
#include "framework/global_env.hpp"

#define GPU_LOADING_HEIGHTMAP

Terrain::Terrain(const std::string& texturePath) : Model() {
    this->LoadHeightMap(texturePath);
    this->VerticesSetup();
}

void Terrain::LoadHeightMap(const std::string& texturePath) {
    std::string heightMapPath = JsonConfigLoader::Read(texturePath, "height_path");
    std::string diffusePath = JsonConfigLoader::Read(texturePath, "diffuse_path");
    std::string specularPath = JsonConfigLoader::Read(texturePath, "specular_path");
    std::string normalPath = JsonConfigLoader::Read(texturePath, "normal_path");
    auto assetsMgr = Global::GetInstance()->GetMgr<AssetsMgr>();
    auto comp = assetsMgr->GetShader("terrain_compute");

    assetsMgr->LoadTexture("terrain_diffuse", diffusePath, TextureType::DIFFUSE);
    assetsMgr->LoadTexture("terrain_specular", specularPath, TextureType::SPECULAR);
    assetsMgr->LoadTexture("terrain_normal", normalPath, TextureType::NORMAL);

    this->textures.push_back(*assetsMgr->GetTexture("terrain_diffuse"));
    this->textures.push_back(*assetsMgr->GetTexture("terrain_specular"));
    this->textures.push_back(*assetsMgr->GetTexture("terrain_normal"));

#ifdef GPU_LOADING_HEIGHTMAP
    assetsMgr->LoadTexture("terrain_height", heightMapPath, TextureType::HEIGHT);

    this->width = assetsMgr->GetTexture("terrain_height")->getWidth();
    this->height = assetsMgr->GetTexture("terrain_height")->getHeight();

    GLuint vertexCount = this->width * this->height;

    comp->use();
    comp->setInt("width", this->width);
    comp->setInt("height", this->height);
    comp->setInt("heightMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, assetsMgr->GetTexture("terrain_height")->id);

    GLuint ssbo[3];
    glGenBuffers(3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[0]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(glm::vec2), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[1]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * vertexCount * sizeof(int), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo[2]);

    glDispatchCompute(this->width / 16, this->height / 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glm::vec4* tmp_vertices = new glm::vec4[vertexCount];
    glm::vec2* tmp_texcoord = new glm::vec2[vertexCount];
    this->indices.resize(2 * this->width * this->height);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec4), tmp_vertices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, vertexCount * sizeof(glm::vec2), tmp_texcoord);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * vertexCount * sizeof(int), indices.data());
    for (int i = 0; i < vertexCount; i++) {
        vertices.emplace_back(tmp_vertices[i], tmp_texcoord[i]);
    }
#else
    unsigned char* heightMap = stbi_load(heightMapPath.c_str(), &this->width, &this->height, &this->nChannels, 0);

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

    for (unsigned int i = 0; i < this->width; i++) {
        for (unsigned int j = 0; j < this->width; j++) {
            for (unsigned int k = 0; k < 2; k++) {
                indices.push_back((i + k) * this->width + j);
            }
        }
    }
#endif
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
