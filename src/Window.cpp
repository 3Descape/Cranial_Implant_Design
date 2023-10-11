#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include "util/util_opengl.hpp"

Window::~Window()
{
    glfwDestroyWindow(window_);
}

int Window::init(float initial_size)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_SAMPLES, 4);

    int work_area_x, work_area_y, work_area_width, work_area_height;
    glfwGetMonitorWorkarea(monitor, &work_area_x, &work_area_y, &work_area_width, &work_area_height);

    int window_width = work_area_width * initial_size;
    int window_height = work_area_height * initial_size;
    int window_x = work_area_x + work_area_width / 2 - window_width / 2;
    int window_y = work_area_y + work_area_height / 2 - window_height / 2;

    window_ = glfwCreateWindow(window_width, window_height, name_.c_str(), NULL, NULL);
    glfwSetWindowMonitor(window_, NULL, window_x, window_y, window_width, window_height, 0);

    if (window_ == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

    glfwSetWindowSizeCallback(window_, window_size_callback_);
    glfwSetWindowPosCallback(window_, window_position_callback_);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback_);
    glfwSetCursorPosCallback(window_, mouse_position_callback_);

    setPosition(window_x, window_y);
    setSize(window_width, window_height);
    setMonitorSize(mode->width, mode->height);

    if (gladLoadGL(glfwGetProcAddress) == 0)
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    OPENGL_CHECK(glViewport(0, 0, window_width, window_height));

    glfwSetWindowUserPointer(window_, this);

    return 0;
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(window_);
}

void default_window_size_callback(GLFWwindow* glfw_window, int width, int height)
{
    Window& window = *(Window*)glfwGetWindowUserPointer(glfw_window);
    window.setSize(width, height);
    if(window.getWindowSizeCallback())
        window.getWindowSizeCallback()(glfw_window, width, height);
}

void default_window_position_callback(GLFWwindow* glfw_window, int x, int y)
{
    Window& window = *(Window*)glfwGetWindowUserPointer(glfw_window);
    window.setPosition(x, y);
    if(window.getWindowPositionCallback())
        window.getWindowPositionCallback()(glfw_window, x, y);
}

void default_framebuffer_size_callback(GLFWwindow* glfw_window, int width, int height)
{
    Window& window = *(Window*)glfwGetWindowUserPointer(glfw_window);
    OPENGL_CHECK(glViewport(0, 0, width, height)); // TODO: check
    if(window.getFramebufferSizeCallback())
        window.getFramebufferSizeCallback()(glfw_window, width, height);
}

void default_mouse_position_callback(GLFWwindow* glfw_window, double x, double y)
{
    Window& window = *(Window*)glfwGetWindowUserPointer(glfw_window);
    window.setMousePosition(x, y);
    if(window.getMousePositionCallback())
        window.getMousePositionCallback()(glfw_window, x, y);
}

GLFWwindow* Window::getGlfwWindow()
{
    return window_;
}

void Window::setWindowSizeCallback(WindowSizeCallback callback)
{
    user_window_size_callback_ = callback;
}

WindowSizeCallback Window::getWindowSizeCallback() const
{
    return user_window_size_callback_;
}

void Window::setWindowPositionCallback(WindowPositionCallback callback)
{
    user_window_position_callback_ = callback;
}

WindowPositionCallback Window::getWindowPositionCallback() const
{
    return user_window_position_callback_;
}

void Window::setFramebufferSizeCallback(FramebufferSizeCallback callback)
{
    user_framebuffer_size_callback_ = callback;
}

FramebufferSizeCallback Window::getFramebufferSizeCallback() const
{
    return user_framebuffer_size_callback_;
}

void Window::setMousePositionCallback(MousePositionCallback callback)
{
    user_mouse_position_callback_ = callback;
}

MousePositionCallback Window::getMousePositionCallback() const
{
    return user_mouse_position_callback_;
}

void Window::setMonitorSize(int width, int height)
{
    monitor_size_.x = width;
    monitor_size_.y = height;
}

glm::vec2 Window::getMonitorSize() const
{
    return monitor_size_;
}

void Window::setPosition(int x, int y)
{
    position_.x = x;
    position_.y = y;
}

glm::vec2 Window::getPosition() const
{
    return position_;
}

void Window::setSize(int width, int height)
{
    size_.x = width;
    size_.y = height;
}

glm::vec2 Window::getSize() const
{
    return size_;
}

void Window::setMousePosition(double x, double y)
{
    mouse_position_.x = x;
    mouse_position_.y = y;
}