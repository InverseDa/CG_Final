//
// Created by inver on 2022/9/25.
//

#ifndef COMPUTERGRAPHICS_TEXTURE_H
#define COMPUTERGRAPHICS_TEXTURE_H

#include "glad/glad.h"
#include "stb/stb_image.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

enum TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT
};

struct Texture {
    unsigned int id;
    TextureType type;
    std::string path;

    Texture() = default;
    Texture(const unsigned int& id);
    Texture(const std::string& path, const TextureType& type);
    // Load Skybox Texture
    Texture(std::vector<std::string>& faces);
    ~Texture() = default;

    // getter
    unsigned int getId() const;
    std::string getType() const;
    std::string getPath() const;

    unsigned int LoadTexture(const std::string& path);
    unsigned int LoadSkyBoxTexture(std::vector<std::string>& faces);
};

#endif // COMPUTERGRAPHICS_TEXTURE_H
