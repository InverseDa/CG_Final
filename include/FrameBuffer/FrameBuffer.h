//
// Created by 缪克达 on 2022/12/25.
//

#ifndef CG_FINAL_FRAMEBUFFER_H
#define CG_FINAL_FRAMEBUFFER_H

#include "glad/glad.h"
#include "iostream"

class FrameBuffer {
public:
    GLuint fbo, rbo;
    // 0: Position; 1: Normal; 2: Diffuse + specular
    GLuint Position, Normal, DiffuseSpecular, DepthTex, WaterTex;
    GLuint DrawBuffers[4];

    bool init(unsigned int WINDOW_WIDTH, unsigned int WINDOW_HEIGHT) {
        // create fbo
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        // create gbuffer texture
        glGenTextures(1, &Position);
        glBindTexture(GL_TEXTURE_2D, Position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Position, 0);
        // normal color buffer
        glGenTextures(1, &Normal);
        glBindTexture(GL_TEXTURE_2D, Normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, Normal, 0);
        // color + specular color buffer
        glGenTextures(1, &DiffuseSpecular);
        glBindTexture(GL_TEXTURE_2D, DiffuseSpecular);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, DiffuseSpecular, 0);
        // gFeatureTexture
        glGenTextures(1, &WaterTex);
        glBindTexture(GL_TEXTURE_2D, WaterTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, WaterTex, 0);
        // depth color buffer
        glGenTextures(1, &DepthTex);
        glBindTexture(GL_TEXTURE_2D, DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                     NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTex, 0);

        DrawBuffers[0] = GL_COLOR_ATTACHMENT0;
        DrawBuffers[1] = GL_COLOR_ATTACHMENT1;
        DrawBuffers[2] = GL_COLOR_ATTACHMENT2;
        DrawBuffers[3] = GL_COLOR_ATTACHMENT3;
        glDrawBuffers(4, DrawBuffers);

//        // create render buffer
//        glGenRenderbuffers(1, &rbo);
//        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
//        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
//
//        // check
//        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//        if (status != GL_FRAMEBUFFER_COMPLETE) {
//            std::cout << "FrameBuffer ERROR: status: " << status << std::endl;
//            return false;
//        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        return true;
    }
};

#endif //CG_FINAL_FRAMEBUFFER_H
