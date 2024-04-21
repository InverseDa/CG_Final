#pragma once

#include "glad/glad.h"
#include <memory>
#include <iostream>
#include <unordered_map>

struct TextureInfo {
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    GLenum attachment;
    bool isDrawBuffer = true;
    std::vector<std::pair<GLenum, GLfloat>> params;
};

class FrameBuffer {
  protected:
    int m_width;
    int m_height;
    GLuint m_fbo;
    GLuint m_rbo;
    std::unordered_map<std::string, GLuint> m_textures;
    std::vector<GLenum> m_drawBuffers;

    static std::shared_ptr<FrameBuffer> CreateFrameBuffer(int width, int height, std::unordered_map<std::string, TextureInfo> attachments);

  public:
    struct Builder {
        int width;
        int height;
        std::unordered_map<std::string, TextureInfo> attachments;

        Builder(int width, int height) : width(width), height(height) {}
        Builder& SetAttachment(const std::string& name,
                               GLenum internalFormat,
                               GLenum format,
                               GLenum type,
                               GLenum attachment,
                               bool isDrawBuffer = true,
                               std::vector<std::pair<GLenum, GLfloat>> params = {
                                   {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                                   {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
                               }) {
            this->attachments[name] = {internalFormat, format, type, attachment, isDrawBuffer, params};
            return *this;
        }
        std::shared_ptr<FrameBuffer> Build() {
            return FrameBuffer::CreateFrameBuffer(width, height, attachments);
        }
    };

    FrameBuffer(int width, int height, std::unordered_map<std::string, TextureInfo> attachments);
    ~FrameBuffer();
    void bind();
    void unbind();
    void resize(int width, int height);
    GLuint getTexture(std::string name) const { return m_textures.at(name); };
};