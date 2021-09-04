#include "Skull.hpp"
#include "Ray.hpp"
#include "AreaPicker.hpp"
#include "Application.hpp"
#include "util/util_mesh.hpp"

void Skull::draw(const glm::mat4& view, const glm::mat4& projection)
{
    bool show_debug = isTarget();
    mesh_.setShader(Shader::get("skull"));

    if(show_debug)
        mesh_.setShader(Shader::get("skull_debug"));

    mesh_.getShader()->use();
    updateModelMatrix();

    mesh_.updateUniforms({
        {"u_model", UniformType::MAT4, (void*)&model_, 1},
        {"u_view", UniformType::MAT4, (void*)&view, 1},
        {"u_projection", UniformType::MAT4, (void*)&projection, 1},
        {"u_color", UniformType::VEC4, (void*)&color_, 1},
    });

    if(show_debug)
    {
        const float picker_radius = app.area_picker->getRadius();
        const glm::vec3 picker_position = xyzToxzy(app.area_picker->getTranslation());
        const int32_t clipping_mode = app.icp_settings.icp_type.value;
        const uint32_t user_selection_points_count = app.user_selection.size();
        const float user_selection_radius = app.selection_radius;

        std::vector<glm::vec3> user_selection_points(user_selection_points_count + 1);
        for(int i = 0; i < user_selection_points_count; ++i) {
            user_selection_points[i] = xyzToxzy(app.user_selection[i].point);
        }

        mesh_.updateUniforms({
            {"u_area_picker_radius", UniformType::FLOAT, (void*)&picker_radius},
            {"u_area_picker_position", UniformType::VEC3, (void*)&picker_position},
            {"u_clipping_mode", UniformType::INT, (void*)&clipping_mode},
            {"u_user_selection_show", UniformType::BOOL, (void*)&app.show_user_selection},
            {"u_region_selection_show", UniformType::BOOL, (void*)&app.show_region_selection},
            {"u_user_selection_radius", UniformType::FLOAT, (void*)&user_selection_radius},
            {"u_user_selection_points_count", UniformType::UINT, (void*)&user_selection_points_count},
        });

        if(user_selection_points_count > 0) { // to avoid setting a nullptr
            mesh_.getShader()->setUniform("u_user_selection_points", UniformType::VEC3, (void*)user_selection_points.data(), user_selection_points_count);
        }
    }

    if(is_wireframe_) OPENGL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    mesh_.draw();
    if(is_wireframe_) OPENGL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

IntersectionPoint Skull::intersect(const Ray& ray)
{
    IntersectionPoint intersection_point(false);
    for(const glm::uvec3& tri : triangles_)
    {
        IntersectionPoint p = rayTriangleIntersect(ray, BarycentricTriangle(points_[tri.x], points_[tri.y], points_[tri.z]));
        if(p.is_valid && p.t < intersection_point.t && p.t >= 0)
            intersection_point = p;
    }

    return intersection_point;
}

void Skull::setMeshData(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles)
{
    points_ = points;
    triangles_ = triangles;
    mask_.resize(points_.size(), false);

    size_t points_size = sizeof(glm::vec3) * points_.size();
    size_t triangles_size = sizeof(glm::uvec3) * triangles_.size();
}

void Skull::updateSelectionMaskWithUserSelection()
{
    mask_active_points_ = 0;
    for(size_t i = 0; i < points_.size(); ++i)
    {
        mask_[i] = false;
        for(const IntersectionPoint& intersection : app.user_selection)
        {
            if(glm::length(intersection.point - points_[i]) <= app.selection_radius)
            {
                mask_active_points_++;
                mask_[i] = true;
                break;
            }
        }
    }
}

void Skull::updateSelectionMaskWithAreaPicker()
{
    AreaPicker* picker = app.area_picker;
    mask_active_points_ = 0;
    for(size_t i = 0; i < points_.size(); ++i)
    {
        mask_[i] = false;

        if(glm::length(picker->getTranslation() - points_[i]) <= picker->getRadius())
        {
            mask_active_points_++;
            mask_[i] = true;
        }
    }
}

const std::vector<bool>& Skull::getMaskReference() const
{
    return mask_;
}

size_t Skull::getMaskActiveCount() const
{
    return mask_active_points_;
}

const std::vector<glm::vec3>& Skull::getPoints() const
{
    return points_;
}

const std::vector<glm::uvec3>& Skull::getTriangles() const
{
    return triangles_;
}

bool Skull::isTarget() const
{
    Skull* target = app.getTargetObject();
    return target != nullptr && target == this;
}