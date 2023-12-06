//
// Created by inver on 2022/9/29.
//

#ifndef COMPUTERGRAPHICS_MODEL_H
#define COMPUTERGRAPHICS_MODEL_H

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "opengl_ext/mesh.hpp"
#include "opengl_ext/model.hpp"
#include "stb/stb_image.h"

class AssimpModel : public Model {
  public:
    /*  模型数据  */
    std::string directory;
    bool gammaCorrection{};

    /*  函数   */
    AssimpModel(const std::string& path) : Model() {
        loadModel(path);
    }

    AssimpModel() : Model() {
    }

    void load(const std::string& path) {
        loadModel(path);
    }

  private:
    /*  函数   */
    void loadModel(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const TextureType& textureType);
};

#endif // COMPUTERGRAPHICS_MODEL_H
