//
// Created by inver on 2022/9/16.
//

#include "Camera.h"

//请提供以下参数：位置信息、z坐标系向量、y坐标系向量、仰视角、平视角、视野
Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, float pitch, float yaw, float fov) {
    this->cameraPos = cameraPos;
    this->cameraFront = cameraFront;
    this->cameraUp = cameraUp;
    this->cameraRight = glm::cross(cameraFront, cameraUp);
    this->pitch = pitch;
    this->yaw = yaw;
    this->fov = fov;
}

void Camera::processMouseMovement(double xOffset, double yOffset) {
    //  灵敏度
    float sensitive = 0.2f;
    xOffset *= sensitive;
    yOffset *= sensitive;

    yaw += xOffset;
    pitch += yOffset;

    //  限制旋转角度
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front = glm::vec3(1.0f);
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
}

void Camera::processScrollMovement(double yOffset) {
    if (fov >= 1.0f && fov <= 45.0f) fov -= yOffset * 5.0f;
    if (fov <= 1.0f) fov = 1.0f;
    if (fov >= 45.0f) fov = 45.0f;
}

glm::mat4 Camera::getView() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::vec3 Camera::getCameraPos() {
    return cameraPos;
}
