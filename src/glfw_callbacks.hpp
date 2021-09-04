typedef struct GLFWwindow GLFWwindow;

#ifndef GLFW_CALLBACKS_H
#define GLFW_CALLBACKS_H

// void window_size_callback(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void window_position_callback(GLFWwindow* window, int xpos, int ypos);
void mouse_position_callback(GLFWwindow* window, double x, double y);

#endif // GLFW_CALLBACKS_H