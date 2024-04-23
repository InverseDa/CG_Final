#include <opengl_ext/camera.hpp>
#include "framework/global_env.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov)
    : cameraPos(position),
      cameraFront(glm::vec3(0.0f, 0.0f, 1.0f)),
      cameraUp(up),
      yaw(yaw),
      pitch(pitch),
      fov(fov) {
    cameraRight = glm::cross(cameraFront, cameraUp);
}

std::shared_ptr<Camera> Camera::CreateCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov) {
    return std::make_shared<Camera>(position, up, yaw, pitch, fov);
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraFront * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraFront * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += glm::vec3{.0f, 1.f, 0.f} * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= glm::vec3{.0f, 1.f, 0.f} * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += 0.1f;

    cameraFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    cameraFront.y = sin(glm::radians(pitch));
    cameraFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(cameraFront);
}

void Camera::processMouseMovement(GLFWwindow* window, float deltaTime) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (isFirstMouse) {
        lastX = xpos;
        lastY = ypos;
        isFirstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // 左右移动
        float yaw = xoffset * 0.01f;
        glm::vec3 rightDir{cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))};
        cameraPos += rightDir * xoffset * 0.01f;
    } else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            xoffset *= 0.1f;
            yoffset *= 0.1f;

            yaw += xoffset;
            pitch += yoffset;

            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            glm::vec3 front = glm::vec3(1.f);
            front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
            front.y = sin(glm::radians(pitch));
            front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

            cameraFront = glm::normalize(front);
        } else {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetCursorPos(window, lastX, lastY);
            }
        }
    }
}

void Camera::processMouseScroll(float yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

