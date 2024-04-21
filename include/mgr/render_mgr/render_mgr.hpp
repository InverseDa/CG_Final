#pragma once
#include "framework/global_env.hpp"
#include "opengl_ext/buffer.hpp"
#include "mgr/camera_mgr/camera_mgr.hpp"
#include "model/quad.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

class RenderMgr {
  public:
    RenderMgr();
    ~RenderMgr() = default;
    static std::shared_ptr<RenderMgr> GetInstance();

    void Run();

  private:
    std::shared_ptr<FrameBuffer> gbuffer;
    std::shared_ptr<FrameBuffer> composite1;
    std::shared_ptr<FrameBuffer> composite2;
    std::shared_ptr<FrameBuffer> shadow;
    Quad screen{};

    std::vector<std::pair<GLenum, GLfloat>> GetDepthTextureParams();
    void CompositePass();
    void GBufferPass();
    void FinalPass();
    void ShadowPass();
};