#pragma once

#include <glad/glad.h>
#include <memory>
#include <opengl_ext/glm.hpp>
#include "GLFW/glfw3.h"

enum DIRECTION {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

class Camera {
  private:
    friend class CameraMgr;
    // Camera Attributes
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    // Euler Angles
    float yaw;
    float pitch;
    // Camera options
    float fov;

    bool isFirstMouse{true};
    float lastX = 0.0f;
    float lastY = 0.0f;
    float movementSpeed = 2.f;

  public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f,
           float fov = 45.0f);
    static std::shared_ptr<Camera>
    CreateCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
                 glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                 float yaw = -90.0f,
                 float pitch = 0.0f,
                 float fov = 45.0f);
    void processKeyboard(GLFWwindow* window, float deltaTime);
    void processMouseMovement(GLFWwindow* window, float deltaTime);
    void processMouseScroll(float yoffset);
    glm::mat4 getViewMatrix() const { return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); }
    glm::mat4 getInverseViewMatrix() const { return glm::inverse(getViewMatrix()); }
    float getFov() const { return fov; }
};