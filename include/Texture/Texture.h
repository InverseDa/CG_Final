//
// Created by inver on 2022/9/25.
//

#ifndef COMPUTERGRAPHICS_TEXTURE_H
#define COMPUTERGRAPHICS_TEXTURE_H
#include <string>

unsigned int loadTexture(const std::string& path);
unsigned int loadSkyBox(std::vector<std::string> &faces);

#endif //COMPUTERGRAPHICS_TEXTURE_H
