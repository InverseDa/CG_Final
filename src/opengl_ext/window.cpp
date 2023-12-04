#include "opengl/glfw.hpp"

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

WindowWrapper::WindowWrapper(int width,
                             int height,
                             std::string&& title,
                             int major_version,
                             int minor_version,
                             Type type)
    : m_width(width),
      m_height(height),
      m_title(title),
      window(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr),
             [](GLFWwindow* window) { glfwDestroyWindow(window); }) {
    m_type = type;
    m_version =
        std::to_string(major_version) + "." + std::to_string(minor_version);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }

    WindowWrapper::makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD.");
    }
}

std::shared_ptr<WindowWrapper> WindowWrapper::createWindow(int width,
                                                           int height,
                                                           std::string&& title,
                                                           int major_version,
                                                           int minor_version,
                                                           Type type) {
    Init::getInstance(major_version, minor_version, type);
    auto wrapper = std::make_shared<WindowWrapper>(
        width, height, std::move(title), major_version, minor_version, type);
    return wrapper;
}

void WindowWrapper::mainLoop(std::function<void()> callback) const {
    makeContextCurrent();
    while (!shouldClose()) {
        callback();
        swapBuffers();
        pollEvents();
    }
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

