#include "mgr/camera_mgr/camera_mgr.hpp"

static std::shared_ptr<CameraMgr> instance = nullptr;
static std::once_flag singletonFlag;

std::shared_ptr<CameraMgr> CameraMgr::GetInstance(glm::vec3 position, float fov) {
    std::call_once(singletonFlag, [&] {
        instance = std::make_shared<CameraMgr>(position, fov);
    });
    return instance;
}

CameraMgr::CameraMgr(glm::vec3 position, float fov) {
    const int width = JsonConfigLoader::Read("env/settings.json", "width");
    const int height = JsonConfigLoader::Read("env/settings.json", "height");
    this->near = 0.1f;
    this->far = 100000.0f;
    this->camera = Camera::CreateCamera(position, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, fov);
    this->projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), this->near, this->far);
}

void CameraMgr::ProcessKeyboardMovement(GLFWwindow* window, float deltaTime) {
    this->camera->processKeyboard(window, deltaTime);
}

void CameraMgr::ProcessMouseMovement(GLFWwindow* window, float deltaTime) {
    this->camera->processMouseMovement(window, deltaTime);
}

void CameraMgr::ProcessMouseScroll(float yoffset) {
    this->camera->processMouseScroll(yoffset);
}
