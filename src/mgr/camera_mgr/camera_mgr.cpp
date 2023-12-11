#include "mgr/camera_mgr/camera_mgr.hpp"

CameraMgr::CameraMgr(glm::vec3 position, float fov) {
    const int width = JsonConfigLoader::Read("env/settings.json", "width");
    const int height = JsonConfigLoader::Read("env/settings.json", "height");
    this->camera = Camera::CreateCamera(position, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, fov);
    this->projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100000.0f);
}

std::shared_ptr<Camera> CameraMgr::GetCamera() const {
    return this->camera;
}

void CameraMgr::ProcessKeyboard(int direction, float deltaTime) {
    this->camera->processKeyboard(direction, deltaTime);
}

void CameraMgr::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    this->camera->processMouseMovement(xoffset, yoffset, constrainPitch);
}

void CameraMgr::ProcessMouseScroll(float yoffset) {
    this->camera->processMouseScroll(yoffset);
}

glm::mat4 CameraMgr::GetViewMatrix() const {
    return this->camera->getViewMatrix();
}

glm::mat4 CameraMgr::GetProjectionMatrix() const {
    return this->projection;
}

glm::mat4 CameraMgr::GetLightSpaceMatrix() const {
    return this->litsrcProj;
}