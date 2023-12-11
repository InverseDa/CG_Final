#pragma once
#include "framework/config_loader.hpp"
#include "opengl_ext/camera.hpp"

class CameraMgr {
  private:
    std::shared_ptr<Camera> camera;
    glm::mat4 projection;
    glm::mat4 litsrcProj;

  public:
    CameraMgr(glm::vec3 position, float fov);
    ~CameraMgr();
    void ProcessKeyboard(int direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);
    void ProcessMouseScroll(float yoffset);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetLightSpaceMatrix() const;
    std::shared_ptr<Camera> GetCamera() const;
};