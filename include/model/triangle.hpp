#ifndef CG_FINAL_TRIANGLE_HPP
#define CG_FINAL_TRIANGLE_HPP
#include "opengl_ext/model.hpp"

class Triangle : public Model {
  private:
    void VerticesSetup(const std::string& jsonPath) {
        std::vector<std::vector<float>> vertices = JsonConfigLoader::Read(jsonPath, "vertices");
        this->vertices.reserve(vertices.size());
        for (const auto& vertex : vertices) {
            this->vertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
        }
        // vao
        glGenVertexArrays(1, &this->VAO);
        glBindVertexArray(this->VAO);
        // vbo
        glGenBuffers(1, &this->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(glm::vec3), this->vertices.data(), GL_STATIC_DRAW);
        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    std::vector<glm::vec3> vertices;
    GLuint VAO, VBO;

  public:
    Triangle(const std::string& jsonPath) : Model() {
        VerticesSetup(jsonPath);
    }
    ~Triangle() = default;

    void Draw(Shader& shader) {
        shader.use();
        glBindVertexArray(this->VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }
};
#endif // CG_FINAL_TRIANGLE_HPP
