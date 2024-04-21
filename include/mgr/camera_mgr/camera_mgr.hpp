#pragma once
#include <mutex>
#include "framework/config_loader.hpp"
#include "opengl_ext/camera.hpp"

class CameraMgr {
  private:
    std::shared_ptr<Camera> camera;
    glm::mat4 projection;
    glm::mat4 litsrcProj;
    float near;
    float far;

  public:
    static std::shared_ptr<CameraMgr> GetInstance(glm::vec3 position = {-20.f, 60.f, -20.f}, float fov = 60.f);

    CameraMgr(glm::vec3 position, float fov);
    ~CameraMgr() = default;
    void ProcessKeyboardMovement(GLFWwindow* window, float deltaTime);
    void ProcessMouseMovement(GLFWwindow* window, float deltaTime);
    void ProcessMouseScroll(float yoffset);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetLightSpaceMatrix() const;
    std::shared_ptr<Camera> GetCamera() const;
    float GetNear() const { return near; }
    float GetFar() const { return far; }
    glm::vec3 GetCameraPosition() const { return camera->cameraPos; }
};