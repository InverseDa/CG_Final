#include "opengl_ext/window.hpp"
#include <iostream>

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

// Path: src/glfw.cpp
Init::Init(int major_version, int minor_version, Type type) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW.");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   type == CORE ? GLFW_OPENGL_CORE_PROFILE
                                : GLFW_OPENGL_COMPAT_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

Init& Init::getInstance(int major_version, int minor_version, Type type) {
    static Init instance(major_version, minor_version, type);
    return instance;
}

Init::~Init() { glfwTerminate(); }

WindowWrapper::WindowWrapper(int width, int height, std::string& title, int major_version, int minor_version, Type type)
    : m_width(width),
      m_height(height),
      m_title(title),
      window(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr),
             [](GLFWwindow* window) { glfwDestroyWindow(window); }) {
    m_type = type;
    m_version = std::to_string(major_version) + "." + std::to_string(minor_version);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }

    WindowWrapper::makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD.");
    }
    // 注册类似Vulkan的Validation回掉
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}

std::shared_ptr<WindowWrapper> WindowWrapper::createWindow(int width,
                                                           int height,
                                                           std::string&& title,
                                                           int major_version,
                                                           int minor_version,
                                                           Type type) {
    Init::getInstance(major_version, minor_version, type);
    auto wrapper = std::make_shared<WindowWrapper>(width, height, title, major_version, minor_version, type);
    return wrapper;
}

WindowWrapper::~WindowWrapper() { // TODO: 关闭窗口
}

GLFWwindow* WindowWrapper::get() const { return window.get(); }

bool WindowWrapper::shouldClose() const {
    return glfwWindowShouldClose(window.get());
}

void WindowWrapper::makeContextCurrent() const {
    glfwMakeContextCurrent(window.get());
}

void WindowWrapper::setKeyCallback(GLFWkeyfun callback) {
    glfwSetKeyCallback(window.get(), callback);
}

void WindowWrapper::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    glfwSetMouseButtonCallback(window.get(), callback);
}

void WindowWrapper::setCursorPosCallback(GLFWcursorposfun callback) {
    glfwSetCursorPosCallback(window.get(), callback);
}

void WindowWrapper::setScrollCallback(GLFWscrollfun callback) {
    glfwSetScrollCallback(window.get(), callback);
}

void WindowWrapper::setFramebufferSizeCallback(
    GLFWframebuffersizefun callback) {
    glfwSetFramebufferSizeCallback(window.get(), callback);
}

void WindowWrapper::setWindowCloseCallback(GLFWwindowclosefun callback) {
    glfwSetWindowCloseCallback(window.get(), callback);
}

void WindowWrapper::setWindowRefreshCallback(GLFWwindowrefreshfun callback) {
    glfwSetWindowRefreshCallback(window.get(), callback);
}

void WindowWrapper::setWindowFocusCallback(GLFWwindowfocusfun callback) {
    glfwSetWindowFocusCallback(window.get(), callback);
}

void WindowWrapper::setWindowIconifyCallback(GLFWwindowiconifyfun callback) {
    glfwSetWindowIconifyCallback(window.get(), callback);
}

void WindowWrapper::setWindowMaximizeCallback(GLFWwindowmaximizefun callback) {
    glfwSetWindowMaximizeCallback(window.get(), callback);
}

void WindowWrapper::setWindowContentScaleCallback(
    GLFWwindowcontentscalefun callback) {
    glfwSetWindowContentScaleCallback(window.get(), callback);
}

void WindowWrapper::setWindowPosCallback(GLFWwindowposfun callback) {
    glfwSetWindowPosCallback(window.get(), callback);
}

void WindowWrapper::setWindowSizeCallback(GLFWwindowsizefun callback) {
    glfwSetWindowSizeCallback(window.get(), callback);
}

void WindowWrapper::setWindowAspectRatioCallback(GLFWwindowsizefun callback) {
    glfwSetWindowSizeCallback(window.get(), callback);
}

void WindowWrapper::setInputMode(int mode, int value) {
    glfwSetInputMode(window.get(), mode, value);
}

int WindowWrapper::getWidth() const { return m_width; }

int WindowWrapper::getHeight() const { return m_height; }

std::string WindowWrapper::getTitle() const { return m_title; }

std::string WindowWrapper::getVersion() const { return m_version; }

std::string WindowWrapper::getType() const {
    return m_type == CORE ? "CORE" : "COMPAT";
}

void WindowWrapper::swapBuffers() const { glfwSwapBuffers(window.get()); }

void WindowWrapper::pollEvents() const { glfwPollEvents(); }
