#pragma once
#include "framework/global_env.hpp"
#include "opengl_ext/buffer.hpp"
#include "mgr/camera_mgr/camera_mgr.hpp"

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
    std::shared_ptr<GBuffer> gbuffer;
    std::shared_ptr<FrameBuffer> composite;
    std::shared_ptr<ShadowBuffer> shadow;

    void CompositePass();
    void GBufferPass();
    void LightPass();
    void ShadowPass();
};