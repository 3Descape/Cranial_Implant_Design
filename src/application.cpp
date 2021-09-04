#include <iostream>
#include <thread>
#include <cstdint>
#include <iterator>
#include <limits>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <openvdb/openvdb.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tools/GridTransformer.h>

#include "Application.hpp"
#include "glfw_callbacks.hpp"
#include "viewlayer/viewlayer.hpp"
#include "resource/NrrdResource.hpp"
#include "resource/MeshResource.hpp"
#include "scene_object/Skull.hpp"
#include "scene_object/LineObject.hpp"
#include "scene_object/AreaPicker.hpp"
#include "scene_object/IntersectionPointObject.hpp"
#include "Ray.hpp"
#include "util/util_ui.hpp"
#include "util/util_opengl.hpp"
#include "util/util_mesh.hpp"
#include "util/util_openvdb.hpp"

Application app;

int Application::init()
{
    glfwInit();

    if(int code = getWindow().init())
        return code;

    getWindow().setMousePositionCallback(mouse_position_callback);
    getWindow().setFramebufferSizeCallback(framebuffer_size_callback);

    OPENGL_CHECK(glGenFramebuffers(1, &fbo_));
    OPENGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));

    OPENGL_CHECK(glGenTextures(1, &color_texture_));
    OPENGL_CHECK(glBindTexture(GL_TEXTURE_2D, color_texture_));
    // TODO: BACKBUFFER
    OPENGL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    OPENGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    OPENGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    OPENGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    OPENGL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    OPENGL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color));
    OPENGL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_, 0));
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return -1;

    GLuint depth_texture;
    OPENGL_CHECK(glGenTextures(1, &depth_texture));
    OPENGL_CHECK(glBindTexture(GL_TEXTURE_2D, depth_texture));
    // TODO: BACKBUFFER
    OPENGL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
    OPENGL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0));

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return -1;
    OPENGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    if(int code = viewlayer_init())
        return code;

    NFD_Init();

    if(int code = createShaders())
        return code;

    if(int code = createCoordinateAxis())
        return code;

    if(int code = createAreaPicker())
        return code;

    IntersectionPointObject::createInstance();

    std::cout << Resource::getDataRootDirectory().string() << std::endl;

    return 0;
}

Application::~Application()
{
    for(auto obj : scene_objects)
        delete obj;

    IntersectionPointObject::destroyInstance();

    Shader::clear();

    NFD_Quit();

    viewlayer_destroy();

    OPENGL_CHECK(glDeleteFramebuffers(1, &fbo_));
    glfwTerminate();
}

void Application::run()
{
	while(!getWindow().shouldClose())
    {
        Timer timer;
        processInput();

        update();

        draw();

        glfwPollEvents();
        int time = (int)timer.stop().milliseconds();
        if(time < 25) {
            std::this_thread::sleep_for(std::chrono::milliseconds(25 - time));
        }
    }
}

void Application::update()
{
    float current_frame = glfwGetTime();
    float delta_time = current_frame - last_frame_;
    last_frame_ = current_frame;

    updateDefaultTargetObjectIfNotSet();

    camera_.setDelta(delta_time);
}

void Application::drawUI()
{
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    viewlayer_draw_objects();
    viewlayer_draw_align();
    viewlayer_draw_vdb();
    viewlayer_draw_cleanup();

     // TODO: BACKBUFFER remove fixed size limitation
    assert(viewport_.content_size.x <= 1920 && "Exceeted max viewport width");
    assert(viewport_.content_size.y <= 1080 && "Exceeted max viewport height");

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    viewport_.begin();
    if(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) || update_cursor_position) {
        ImGuiIO& io = ImGui::GetIO();

        bool is_mouse_hidden = update_cursor_position;
        if(ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            update_cursor_position = true;
            ImVec2 mouse_pos = ImGui::GetMousePos();
            if(is_mouse_hidden == false) {
                camera_.setLastCursorPos(mouse_pos.x, mouse_pos.y);
                glfwSetInputMode(getWindow().getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            app.getCamera().updateRotation(mouse_pos.x, mouse_pos.y);
        }
        else if(is_mouse_hidden == true) {
            update_cursor_position = false;
            ImVec2 mouse_pos;
            camera_.getLastCursorPos(mouse_pos.x, mouse_pos.y);
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
            glfwSetCursorPos(getWindow().getGlfwWindow(), mouse_pos.x, mouse_pos.y);
            glfwSetInputMode(getWindow().getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if(ImGui::IsKeyDown(ImGuiKey_W)) {
            camera_.moveForward();
        }

        if(ImGui::IsKeyDown(ImGuiKey_S)) {
            camera_.moveBackward();
        }

        if(ImGui::IsKeyDown(ImGuiKey_A)) {
            camera_.moveLeft();
        }

        if(ImGui::IsKeyDown(ImGuiKey_D)) {
            camera_.moveRight();
        }

        if((ImGui::IsKeyDown(ImGuiKey_Space) && !ImGui::IsKeyDown(ImGuiKey_LeftShift)) || ImGui::IsKeyDown(ImGuiKey_Q)) {
            camera_.moveUp();
        }

        if((ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsKeyDown(ImGuiKey_LeftShift)) || ImGui::IsKeyDown(ImGuiKey_E)) {
            camera_.moveDown();
        }
    }
    ImGui::End();
    viewport_.update();
    viewport_.drawFullScreenImage((ImTextureID)uint64_t(color_texture_), 1920, 1080);
    ImGui::PopStyleVar();
}

bool Application::selectedObjectIsValidAlignSource() const
{
    return  selected_object_index != -1 &&
            target_index != selected_object_index &&
            scene_objects[selected_object_index]->getType() == SceneObject::SKULL;
}

bool Application::isValidAlignTarget() const
{
    return target_index != -1 && scene_objects[target_index]->getType() == SceneObject::SKULL;
}

std::vector<SceneObject*> Application::getVisibleAlignSources() const
{
    std::vector<SceneObject*> objects;

    for(int i = 0; i < scene_objects.size(); ++i)
    {
        if(i != target_index && scene_objects[i]->isVisible() && scene_objects[i]->getType() == SceneObject::SKULL)
            objects.push_back(scene_objects[i]);
    }

    return objects;
}

std::vector<SceneObject*> Application::getAlignSources() const
{
    std::vector<SceneObject*> objects;

    for(int i = 0; i < scene_objects.size(); ++i)
    {
        if(i != target_index && scene_objects[i]->getType() == SceneObject::SKULL)
            objects.push_back(scene_objects[i]);
    }

    return objects;
}

void Application::drawScene()
{
    if(viewport_.content_size.x == 0 && viewport_.content_size.y == 0)
        return;

    OPENGL_CHECK(glEnable(GL_BLEND));
    OPENGL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    OPENGL_CHECK(glViewport(0, 0, viewport_.content_size.x, viewport_.content_size.y));
    OPENGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));

    OPENGL_CHECK(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
    OPENGL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    OPENGL_CHECK(glEnable(GL_DEPTH_TEST));
    // OPENGL_CHECK(glEnable(GL_MULTISAMPLE));

    glm::mat4 view = camera_.getLookAt();
    glm::mat4 projection = camera_.getProjection(viewport_.content_size.x / viewport_.content_size.y);

    scene_objects_lock.lock();
    for(auto scene_object : scene_objects)
    {
        if(!scene_object->isVisible() || scene_object->getType() == SceneObject::AREA_PICKER) continue;
        scene_object->draw(view, projection);
    }
    scene_objects_lock.unlock();

    IntersectionPoint intersection(false);
    int ctrl_pressed = glfwGetKey(getWindow().getGlfwWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

    // scene ray picking
    Skull* target = getTargetObject();
    if((target != nullptr) && ctrl_pressed)
    {
        float x, y;
        viewport_.getNormalizedCursorPosition(x, y);
        glm::vec3 cursor_pos = viewToWorldSpace(x, y, camera_.getProjection(viewport_.content_size.x / viewport_.content_size.y) * camera_.getLookAt());
        glm::vec3 camera_pos = camera_.getPosition();
        Ray ray(camera_pos, glm::normalize(cursor_pos - camera_pos));
        intersection = target->intersect(ray);
    }

    static int old_state_lmb = GLFW_RELEASE;
    static int old_state_rmb = GLFW_RELEASE;
    int new_state_lmb = glfwGetMouseButton(getWindow().getGlfwWindow(), GLFW_MOUSE_BUTTON_LEFT);
    int new_state_rmb = glfwGetMouseButton(getWindow().getGlfwWindow(), GLFW_MOUSE_BUTTON_RIGHT);

    bool mouse_capture_intersection = false;
    IntersectionPointObject obj;
    for(size_t i = 0; i < user_selection.size(); ++i)
    {
        auto& selection = user_selection[i];
        obj.setHighlight(false);

        if(glm::length(selection.point - intersection.point) <= 0.5f && ctrl_pressed)
        {
            mouse_capture_intersection = true;
            obj.setHighlight(true);
            if(old_state_rmb == GLFW_PRESS && new_state_rmb == GLFW_RELEASE)
            {
                std::cout << "Remove point" << std::endl;
                user_selection.erase(user_selection.begin() + i);
                i--;
                continue;
            }

            if(new_state_lmb == GLFW_PRESS)
            {
                user_selection[i].point = intersection.point;
                user_selection[i].normal = intersection.normal;
            }
        }

        obj.getTranslation() = selection.point;
        obj.getRotation() = selection.normal;
        obj.draw(view, projection);
    }

    if(intersection.is_valid && !mouse_capture_intersection && ctrl_pressed)
    {
        IntersectionPointObject obj;
        obj.getTranslation() = intersection.point;
        obj.getRotation() = intersection.normal;
        obj.updateModelMatrix();
        obj.draw(view, projection);

        if(old_state_lmb == GLFW_PRESS && new_state_lmb == GLFW_RELEASE)
        {
            std::cout << "added point at x: " << intersection.point.x << ", y: " << intersection.point.y << ", z: " << intersection.point.z  << std::endl;
            user_selection.push_back(intersection);
        }
    }

    old_state_lmb = new_state_lmb;
    old_state_rmb = new_state_rmb;

    if(area_picker->isVisible()) {
        area_picker->draw(view, projection);
    }

    OPENGL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    OPENGL_CHECK(glDisable(GL_BLEND));
}

void Application::draw()
{
    OPENGL_CHECK(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
    OPENGL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    viewlayer_start();
    drawScene();
    drawUI();
    viewlayer_end();

    glfwSwapBuffers(getWindow().getGlfwWindow());
}

int Application::createCoordinateAxis()
{
    constexpr float coordiante_axis_length = 1000.0f;
    constexpr uint32_t coordiante_axis_subdivisions = 0;
    constexpr float coordinate_axis_width = 0.1f;

    std::vector<float> data;
    std::vector<glm::uvec2> indices;
    data.reserve((coordiante_axis_subdivisions + 2) * 2);
    indices.reserve(coordiante_axis_subdivisions + 2);

    size_t vertex_index = 0;
    for(int k = 0; k < 3; ++k)
    {
        bool first = true;
        for(float i = -coordiante_axis_length / 2; i <= coordiante_axis_length; i += (coordiante_axis_length / (coordiante_axis_subdivisions + 1)))
        {
            const float x = (k == 0 ? i : 0);
            const float y = (k == 1 ? i : 0);
            const float z = (k == 2 ? i : 0);
            data.push_back(x);
            data.push_back(y);
            data.push_back(z);
            const float red = k == 0 ? 1.0f : 0.0f;
            const float green = k == 1 ? 1.0f : 0.0f;
            const float blue = k == 2 ? 1.0f : 0.0f;
            data.push_back(red);
            data.push_back(green);
            data.push_back(blue);

            if(!first)
                indices.push_back(glm::uvec2(vertex_index - 1, vertex_index));

            first = false;

            vertex_index++;
        }
    }

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::AoS,
        data.data(),
        data.size() * sizeof(data[0]),
        indices.data(),
        indices.size() * sizeof(indices[0]),
        indices.size() * 2,
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float) },
            { "color", AttributeLayout::COLOR, 3, GL_FLOAT, sizeof(float) },
        },
        Shader::get("3D_position_color"),
    };

    LineObject* coordinate_axis = new LineObject("Coordinate Axis", descriptor);
    addSceneObject(coordinate_axis);

    return 0;
}

int Application::createAreaPicker()
{
    area_picker = new AreaPicker("Region Picker");
    addSceneObject(area_picker);

    icp_settings.area_picker = area_picker;

    return 0;
}

int Application::createShaders()
{
    const std::vector<Shader::ShaderSources> shader_sources {
        {"3D_flat", "./shaders/flat_vert.glsl", "./shaders/flat_frag.glsl"},
        {"3D_position_color", "./shaders/color_vert.glsl", "./shaders/color_frag.glsl"},
        {"thresholded_voxel_visualization", "./shaders/thresholded_voxel_visualization_vert.glsl", "./shaders/thresholded_voxel_visualization_frag.glsl"},
        {"flat_random_line", "./shaders/flat_random_line_vert.glsl", "./shaders/flat_random_frag.glsl"},
        {"flat_random_point", "./shaders/flat_random_point_vert.glsl", "./shaders/flat_random_frag.glsl"},
        {"grid_visualization", "./shaders/grid_visualization_vert.glsl", "./shaders/grid_visualization_frag.glsl"},
        {"3D_position_normal", "./shaders/diffuse_vert.glsl", "./shaders/diffuse_frag.glsl"},
        {"intersectionpoint", "./shaders/intersectionpoint_vert.glsl", "./shaders/intersectionpoint_frag.glsl"},
        {"skull", "./shaders/skull_vert.glsl", "./shaders/skull_frag.glsl", "./shaders/skull_debug_geo.glsl"},
        {"skull_debug", "./shaders/skull_debug_vert.glsl", "./shaders/skull_debug_frag.glsl", "./shaders/skull_debug_geo.glsl"},
        {"debug", "./shaders/debug_vert.glsl", "./shaders/debug_frag.glsl"},
    };

    for(const auto& info : shader_sources)
    {
        const char* geometry_shader_path = info.geometry_shader.c_str();
        if(info.geometry_shader.empty())
            geometry_shader_path = nullptr;

        Shader shader(info.name, info.vertex_shader.c_str(), info.fragment_shader.c_str(), geometry_shader_path);
        if (!shader.isValid()) return -1;
        shader.save();
    }

    return 0;
}

void Application::updateDefaultTargetObjectIfNotSet()
{
    if(app.getSkullCount() < 1 || app.target_index != -1) return;

    for(int i = 0; i < app.scene_objects.size(); ++i) {
        if(app.scene_objects.at(i)->getType() == SceneObject::SKULL) {
            app.target_index = i;
        }
    }
}

void Application::addNrrdObjects(const std::vector<boost::filesystem::path>& paths)
{
    for(auto path : paths)
        addNrrdObject(path);
}

void Application::addNrrdObject(const boost::filesystem::path& path)
{
    NrrdResource nrrd_resource(NrrdResource::getRelativePath(path));
    addNrrdObject(nrrd_resource);
}

void Application::addNrrdObject(const NrrdResource& nrrd_resource)
{
    MeshResource mesh_resource(nrrd_resource);
    addMeshObject(mesh_resource);
}

void Application::addMeshObjects(const std::vector<MeshResource>& mesh_resources)
{
    for(auto mesh_r : mesh_resources)
        addMeshObject(mesh_r);
}

void Application::addMeshObject(const MeshResource& mesh_resource)
{
    std::vector<glm::vec3> skull_points;
    std::vector<glm::uvec3> skull_triangles;
    mesh_resource.load(&skull_points, &skull_triangles);

    SoA_Region_List regions = SoA_mesh_from_position(skull_points, skull_triangles);

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        skull_points.data(),
        skull_points.size() * sizeof(skull_points[0]),
        skull_triangles.data(),
        skull_triangles.size() * sizeof(skull_triangles[0]),
        skull_triangles.size() * 3,
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), regions.get("position")->offset },
        },
        nullptr,
        uint32_t(OpenglBufferFlag::ALL),
        regions,
    };

    Skull* skull = new Skull(mesh_resource.getRelativeFilePath().string(), descriptor);
    skull->setMeshData(skull_points, skull_triangles);
    addSceneObject(skull);
    skull_count_++;
}

void Application::processInput()
{
    ImGuiIO& io = ImGui::GetIO();

    if(ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        glfwSetWindowShouldClose(getWindow().getGlfwWindow(), true);
    }

    if(ImGui::IsKeyPressed(ImGuiKey_T)) {
        Skull* target = app.getTargetObject();
        if(target)
            target->setVisibility(target->isVisible() ? SceneObject::HIDDEN : SceneObject::VISIBLE);
    }

    if(ImGui::IsKeyPressed(ImGuiKey_H) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        app.hide_all_source_skulls = !app.hide_all_source_skulls;
        for(auto& target : app.getAlignSources()) {
            target->setVisibility(app.hide_all_source_skulls ? SceneObject::HIDDEN : SceneObject::VISIBLE);
        }
    }

    if(ImGui::IsKeyPressed(ImGuiKey_F1)) {
        show_demo_window = true;
    }
}

Window& Application::getWindow()
{
    return window_;
}

Camera& Application::getCamera()
{
    return camera_;
}

uint32_t Application::getSkullCount()
{
    return skull_count_;
}

Skull* Application::getTargetObject()
{
    if(target_index < 0)
        return nullptr;

    if(scene_objects[target_index]->getType() != SceneObject::SKULL) {
        return nullptr;
    }

    return (Skull*)scene_objects[target_index];
}

SceneObject* Application::getSelectedObject()
{
    if(selected_object_index < 0)
        return nullptr;

    return scene_objects[selected_object_index];
}

void Application::addSceneObject(SceneObject* object)
{
    scene_objects_lock.lock();
    scene_objects.push_back(object);
    scene_objects_lock.unlock();
}

void Application::deleteSceneObject(int64_t index)
{
    std::lock_guard<std::mutex> lock_guard(scene_objects_lock);
    if(scene_objects[index]->getType() == SceneObject::AREA_PICKER) return;
    if(scene_objects[index]->getType() == SceneObject::SKULL) skull_count_--;
    delete scene_objects[index];
    scene_objects.erase(scene_objects.begin() + index);
    if(target_index == index) target_index = -1;
    if(target_index > index) target_index--;

    if(selected_object_index == -1) return;

    if(scene_objects.size() == 0)
        selected_object_index = -1;
    else if(selected_object_index >= scene_objects.size()) // select prev, if object was last in list
        selected_object_index--;
}
