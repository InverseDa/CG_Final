#pragma once

#include "glad/glad.h"
#include <memory>
#include <iostream>

class FrameBuffer {
  protected:
    int m_width;
    int m_height;
    GLuint m_fbo;
    GLuint m_rbo;
    GLuint m_texture;

  public:
    static std::shared_ptr<FrameBuffer> CreateFrameBuffer(int width,
                                                          int height);
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void bind();
    void unbind();

    void resize(int width, int height);

    GLuint getTexture() const;
};

class GBuffer {
  protected:
    int m_width;
    int m_height;
    GLuint m_fbo;
    GLuint m_rbo;
    GLuint m_position;
    GLuint m_normal;
    GLuint m_albedoSpec;

  public:
    static std::shared_ptr<GBuffer> CreateGBuffer(int width, int height);
    GBuffer(int width, int height);
    ~GBuffer();

    void Bind();
    void Unbind();

    void resize(int width, int height);

    GLuint getPosition() const;
    GLuint getNormal() const;
    GLuint getAlbedoSpec() const;
};

class ShadowBuffer {
protected:
    int m_width;
    int m_height;
    GLuint m_fbo;
    GLuint m_rbo;
    GLuint m_shadowMap;

public:
    static std::shared_ptr<ShadowBuffer> CreateShadowBuffer(int width, int height);
    ShadowBuffer(int width, int height);
    ~ShadowBuffer();

    void Bind();
    void Unbind();

    GLuint GetShadowMap();
};