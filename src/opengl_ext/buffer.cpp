#include "opengl_ext/buffer.hpp"

// =================================================================================================
// =================================================================================================
// ======================================== FrameBuffer ============================================
// =================================================================================================
// =================================================================================================

std::shared_ptr<FrameBuffer> FrameBuffer::CreateFrameBuffer(int width, int height, std::vector<std::pair<std::string, TextureInfo>> attachments) {
    return std::make_shared<FrameBuffer>(width, height, attachments);
}

FrameBuffer::FrameBuffer(int width, int height, std::vector<std::pair<std::string, TextureInfo>> attachments)
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
        for (auto& parameter : textureInfo.params) {
            glTexParameteri(GL_TEXTURE_2D, parameter.first, parameter.second);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, textureInfo.attachment, GL_TEXTURE_2D, texture, 0);
        m_textures[name] = texture;
        if (textureInfo.flag) {
            m_drawBuffers.push_back(textureInfo.attachment);
        }
    }

    if (!m_drawBuffers.empty()) {
        glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    //    glGenRenderbuffers(1, &m_rbo);
    //    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    //    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    //
    //    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
    //                  << std::endl;
    //    }
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

void FrameBuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::UnBind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
