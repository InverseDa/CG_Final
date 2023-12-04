#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class Shader {
  private:
    unsigned int id;

  public:
    Shader(const std::string& vertexShaderPath,
           const std::string& fragmentShaderPath);

    Shader(const std::string& glslPath);

    Shader(const char* vertexShaderCode, const char* fragmentShaderCode);

    static std::shared_ptr<Shader>
    createShaderByPath(const std::string& vertexShaderPath,
                       const std::string& fragmentShaderPath);

    static std::shared_ptr<Shader>
    createShaderByPath(const std::string& glslPath);

    static std::shared_ptr<Shader>
    createShaderBySource(const char* vertexShaderCode,
                         const char* fragmentShaderCode);

    void use() const;

    unsigned int getId() const;

    // uniform tools
    void setBool(const std::string& name, bool value) const;

    void setInt(const std::string& name, int value) const;

    void setFloat(const std::string& name, float value) const;

    void setVector2(const std::string& name, float x, float y) const;

    void setVector2(const std::string& name, glm::vec2 vec) const;

    void setVector3(const std::string& name, float x, float y, float z) const;

    void setVector3(const std::string& name, glm::vec3 vec) const;

    void setVector4(const std::string& name, float x, float y, float z, float w) const;

    void setVector4(const std::string& name, glm::vec4 vec) const;

    void setMatrix3(const std::string& name, glm::mat3 mat3) const;

    void setMatrix4(const std::string& name, glm::mat4 mat4) const;
};
