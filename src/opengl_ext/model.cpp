#include "opengl_ext/model.hpp"

void Model::Draw(Shader& shader) {
    for (auto& mesh : this->meshes) {
        mesh.Draw(shader);
    }
}
