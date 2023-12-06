#include "opengl_ext/mesh.hpp"

std::unordered_map<TextureType, std::string> textureTypeString = {
    {TextureType::DIFFUSE, "texture_diffuse"},
    {TextureType::SPECULAR, "texture_specular"},
    {TextureType::NORMAL, "texture_normal"},
    {TextureType::HEIGHT, "texture_height"}
};

Mesh::Mesh(std::vector<Vertex>& vertices,
           std::vector<unsigned int>& indices,
           std::vector<Texture>& textures) {
    this->useEBO = true;
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    SetupMesh();
}

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<Texture>& textures, bool isSkyBox) {
    this->useEBO = false;
    this->vertices = vertices;
    this->textures = textures;
    this->isSkyBox = isSkyBox;

    SetupMesh();
}

void Mesh::Draw(Shader& shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        TextureType type = textures[i].type;
        if (type == TextureType::DIFFUSE)
            number = std::to_string(diffuseNr++);
        else if (type == TextureType::SPECULAR)
            number = std::to_string(specularNr++);
        else if (type == TextureType::NORMAL)
            number = std::to_string(normalNr++);
        else if (type == TextureType::HEIGHT)
            number = std::to_string(heightNr++);

        shader.setInt(textureTypeString[type] + number, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    if (isSkyBox) glDepthFunc(GL_LEQUAL);
    glBindVertexArray(VAO);
    if(useEBO)
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
    if (isSkyBox) glDepthFunc(GL_LESS);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    if (useEBO) glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    if (useEBO) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }

    //  顶点位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)nullptr);

    //  顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    //  顶点纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}