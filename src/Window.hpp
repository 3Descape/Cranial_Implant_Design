#include <string>

#include <glm/glm.hpp>

typedef struct GLFWwindow GLFWwindow;

#ifndef WINDOW_H
#define WINDOW_H

typedef void (*WindowSizeCallback)(GLFWwindow*, int, int);
typedef void (*WindowPositionCallback)(GLFWwindow*, int, int);
typedef void (*FramebufferSizeCallback)(GLFWwindow*, int, int);
typedef void (*MousePositionCallback)(GLFWwindow*, double, double);

void default_window_size_callback(GLFWwindow*, int, int);
void default_window_position_callback(GLFWwindow*, int, int);
void default_framebuffer_size_callback(GLFWwindow*, int, int);
void default_mouse_position_callback(GLFWwindow*, double, double);

class Window
{
    private:
        const std::string name_;
        GLFWwindow* window_ = 0;
        glm::vec2 position_;
        glm::vec2 size_;
        glm::vec2 monitor_size_;
        glm::vec2 mouse_position_;

        const WindowSizeCallback window_size_callback_ = nullptr;
        const WindowPositionCallback window_position_callback_ = nullptr;
        const FramebufferSizeCallback framebuffer_size_callback_ = nullptr;
        const MousePositionCallback mouse_position_callback_ = nullptr;
        WindowSizeCallback user_window_size_callback_ = nullptr;
        WindowPositionCallback user_window_position_callback_ = nullptr;
        FramebufferSizeCallback user_framebuffer_size_callback_ = nullptr;
        MousePositionCallback user_mouse_position_callback_ = nullptr;

    public:
        Window(
            const std::string& name,
            WindowSizeCallback window_size_callback = default_window_size_callback,
            WindowPositionCallback window_position_callback = default_window_position_callback,
            FramebufferSizeCallback framebuffer_size_callback = default_framebuffer_size_callback,
            MousePositionCallback mouse_position_callback = default_mouse_position_callback
        ) :
            name_(name),
            window_size_callback_(window_size_callback),
            window_position_callback_(window_position_callback),
            framebuffer_size_callback_(framebuffer_size_callback),
            mouse_position_callback_(mouse_position_callback) {}

        Window(const Window&) = default;
        Window& operator=(const Window&) = delete;
        ~Window();

        int init(float initial_size = 0.75f);
        bool shouldClose();

        GLFWwindow* getGlfwWindow();
        void setWindowSizeCallback(WindowSizeCallback callback);
        WindowSizeCallback getWindowSizeCallback() const;
        void setWindowPositionCallback(WindowPositionCallback callback);
        WindowPositionCallback getWindowPositionCallback() const;
        void setFramebufferSizeCallback(FramebufferSizeCallback callback);
        FramebufferSizeCallback getFramebufferSizeCallback() const;
        void setMousePositionCallback(MousePositionCallback callback);
        MousePositionCallback getMousePositionCallback() const;
        void setMonitorSize(int width, int height);
        glm::vec2 getMonitorSize() const;
        void setPosition(int x, int y);
        glm::vec2 getPosition() const;
        void setSize(int width, int height);
        glm::vec2 getSize() const;
        void setMousePosition(double x, double y);
};

#endif // WINDOW_H