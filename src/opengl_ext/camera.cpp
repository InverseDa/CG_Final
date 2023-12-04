#include <opengl/camera.hpp>

Camera::Camera(
    glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov)
    : cameraPos(position),
      cameraFront(glm::vec3(0.0f, 0.0f, 1.0f)),
      cameraUp(up),
      yaw(yaw),
      pitch(pitch),
      fov(fov) {
    cameraRight = glm::cross(cameraFront, cameraUp);
}

std::shared_ptr<Camera> Camera::createCamera(
    glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov) {
    return std::make_shared<Camera>(position, up, yaw, pitch, fov);
}

void Camera::processKeyboard(int direction, float deltaTime) {
    float velocity = 2.f * deltaTime;
    if (direction == FORWARD)
        cameraPos += cameraFront * velocity;
    if (direction == BACKWARD)
        cameraPos -= cameraFront * velocity;
    if (direction == LEFT)
        cameraPos -=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    // 飞机模式用：
    // cameraPos -= cameraRight * velocity;
    if (direction == RIGHT)
        cameraPos +=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    // 飞机模式用：
    // cameraPos += cameraRight * velocity;
}

void Camera::processMouseMovement(float xoffset,
                                  float yoffset,
                                  bool constrainPitch) {
    xoffset *= 0.01f;
    yoffset *= 0.01f;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    glm::vec3 front = glm::vec3(1.f);
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    cameraFront = glm::normalize(front);
    // 以下参数一旦开启，那就是飞机模式（可以在空中360度旋转）
    // cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
    // cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void Camera::processMouseScroll(float yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

float Camera::getFov() const { return fov; }
