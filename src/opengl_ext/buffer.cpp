#include "opengl_ext/buffer.hpp"

// =================================================================================================
// =================================================================================================
// ======================================== FrameBuffer ============================================
// =================================================================================================
// =================================================================================================

std::shared_ptr<FrameBuffer> FrameBuffer::CreateFrameBuffer(int width, int height, std::unordered_map<std::string, TextureInfo> attachments) {
    return std::make_shared<FrameBuffer>(width, height, attachments);
}

FrameBuffer::FrameBuffer(int width, int height, std::unordered_map<std::string, TextureInfo> attachments)
    : m_width(width),
      m_height(height) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    for (auto& attachment : attachments) {
        auto& [name, textureInfo] = attachment;
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, textureInfo.internalFormat, m_width, m_height, 0, textureInfo.format, textureInfo.type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST
        glFramebufferTexture2D(GL_FRAMEBUFFER, textureInfo.attachment, GL_TEXTURE_2D, texture, 0);
        m_textures[name] = texture;
    }

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    }
    // reset to default texture and frame buffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteRenderbuffers(1, &m_rbo);
    for (auto& texture : m_textures) {
        glDeleteTextures(1, &texture.second);
    }
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(int width, int height) {
    m_width = width;
    m_height = height;

    for (auto& texture : m_textures) {
        glBindTexture(GL_TEXTURE_2D, texture.second);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}