#ifndef CG_FINAL_QUAD_HPP
#define CG_FINAL_QUAD_HPP
#include "opengl_ext/model.hpp"

class Quad : public Model {
  private:
    void VerticesSetup() {
        this->vertices = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        };
        this->indices = {
            0, 1, 2,
            2, 3, 0,
        };
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

    std::vector<std::pair<glm::vec3, glm::vec2>> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;

  public:
    Quad() : Model() {
        this->VerticesSetup();
    }
    ~Quad() = default;

    void Draw(Shader& shader) {
        shader.use();
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};
#endif // CG_FINAL_QUAD_HPP
