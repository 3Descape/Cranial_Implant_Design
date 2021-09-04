#include <string>

#include "imgui/imgui.h"

#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct Viewport
{
    const std::string name;
    ImVec2 content_area_start; // Screen space of monitor
    ImVec2 content_size;

    Viewport(const std::string& name) : name(name) {}
    Viewport(const Viewport&) = default;
    Viewport& operator=(const Viewport&) = delete;
    ~Viewport() = default;

    void begin()
    {
        ImGui::Begin(name.c_str(), NULL);
    }

    void update()
    {
        begin();

        // TODO: check if the size is correct(offset from regionMin)

        ImVec2 window_min = ImGui::GetWindowContentRegionMin();
        ImVec2 window_max = ImGui::GetWindowContentRegionMax();
        ImVec2 cursor_pos = ImGui::GetCursorPos();
        ImVec2 window_pos = ImGui::GetWindowPos();

        content_area_start = ImVec2(window_pos.x + cursor_pos.x, window_pos.y + cursor_pos.y);
        content_size = ImVec2(window_max.x - window_min.x, window_max.y - window_min.y);

        ImGui::End();

        // ImVec2 mouse_pos = ImGui::GetMousePos();
        // ImVec2 mouse_relative_position = ImVec2(mouse_pos.x - content_area_start.x, mouse_pos.y - content_area_start.y);
        // std::cout << "Min: Window: " << window_min.x << ", " << window_min.y << ",  Cursor: " << cursor_pos.x << ", " << cursor_pos.y << std::endl;
        // std::cout << "Max: Window: " << window_max.x << ", " << window_max.y << std::endl;
        // std::cout << "Mouse      : " << mouse_relative_position.x << ", " <<  mouse_relative_position.y << std::endl;
    }

    void drawFullScreenImage(ImTextureID texture, int resolution_x, int resolution_y) const
    {
        float u = content_size.x / float(resolution_x);
        float v = content_size.y / float(resolution_y);

        ImGui::Begin(name.c_str(), NULL);
        ImGui::Image(texture, content_size, ImVec2{0, v}, ImVec2{u, 0}); // (0, 0) is top left in hlsl vs bottom left in opengl
        ImGui::End();
    }

    void getNormalizedCursorPosition(float& x, float& y) const
    {
        ImVec2 mouse_position = ImGui::GetMousePos();
        x = 2.0f * (mouse_position.x - content_area_start.x) / content_size.x  - 1.0f;
        y = 2.0f * (mouse_position.y - content_area_start.y) / content_size.y  - 1.0f;
    }

} Viewport;

#endif // VIEWPORT_H