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

extern const std::unordered_map<TextureType, std::string> textureTypeString;

struct Texture {
    unsigned int id;
    TextureType type;
    std::string path;

    Texture();
    Texture(const std::string& path, const TextureType& type);
    ~Texture();

    // getter
    unsigned int getId() const;
    std::string getType() const;
    std::string getPath() const;

    unsigned int loadTexture(const std::string& path);
};

unsigned int loadSkyBox(std::vector<std::string>& faces);

#endif // COMPUTERGRAPHICS_TEXTURE_H
