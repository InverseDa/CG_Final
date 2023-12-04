#pragma once

#include <glad/glad.h>
#include <memory>
#include <opengl/glm.hpp>

enum DIRECTION {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

class Camera {
  private:
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

  public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f,
           float fov = 45.0f);
    static std::shared_ptr<Camera>
    createCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
                 glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                 float yaw = -90.0f,
                 float pitch = 0.0f,
                 float fov = 45.0f);
    void processKeyboard(int direction, float deltaTime);
    void processMouseMovement(float xoffset,
                              float yoffset,
                              bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    glm::mat4 getViewMatrix() const;
    float getFov() const;
};