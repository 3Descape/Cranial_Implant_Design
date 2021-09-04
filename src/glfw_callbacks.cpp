#include "Application.hpp"
#include <glad/glad.h>

// void window_size_callback(GLFWwindow* window, int width, int height)
// {
//     if(!app.isFullscreen())
//         app.getWindow().setSize(width, height);
// }

// void window_position_callback(GLFWwindow* window, int xpos, int ypos)
// {
//     if(!app.isFullscreen())
//         app.getWindow().setPosition(xpos, ypos);
// }

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    app.draw();
}

void mouse_position_callback(GLFWwindow* window, double x, double y)
{
}