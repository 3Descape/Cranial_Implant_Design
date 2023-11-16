#include <SoA.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#ifndef UTIL_MESH_H
#define UTIL_MESH_H

inline SoA_Region_List SoA_mesh_from_position(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles)
{
    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(points[0]), points.size() }, SoA_Memcpy_Descriptor::ARRAY,  (void*)points.data() },
    };

    ASSERT_MSG_DEBUG(SoA_get_size_bytes(descriptors) % sizeof(float) == 0, "Remainder was not 0.");

    return SoA_create_region_list(descriptors);
}

inline SoA_Region_List SoA_mesh_from_position_color4_normal(const std::vector<glm::vec3>& points, const std::vector<glm::vec4>& colors, const std::vector<glm::vec3>& normals, std::vector<float>& out_data)
{
    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(points[0]), points.size() }, SoA_Memcpy_Descriptor::ARRAY,  (void*)points.data() },
        { { "color", sizeof(colors[0]), colors.size() }, SoA_Memcpy_Descriptor::ARRAY, (void*)colors.data()},
        { { "normal", sizeof(normals[0]), normals.size() }, SoA_Memcpy_Descriptor::ARRAY, (void*)normals.data() },
    };

    ASSERT_MSG_DEBUG(SoA_get_size_bytes(descriptors) % sizeof(float) == 0, "Remainder was not 0.");
    out_data.resize(SoA_get_size_bytes(descriptors) / sizeof(float));
    SoA_memcpy(out_data.data(), descriptors);

    return SoA_create_region_list(descriptors);
}

inline void mesh_convert_to_flat_normals(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles, std::vector<glm::vec3>& out_points, std::vector<glm::vec3>& out_normals, std::vector<glm::uvec3>& out_triangles)
{
    out_triangles.reserve(triangles.size());
    out_points.reserve(triangles.size() * 3);
    out_normals.reserve(triangles.size() * 3);

    size_t vertex_index = 0;
    for(const glm::uvec3& triangle : triangles)
    {
        auto p1 = points[triangle.x];
        auto p2 = points[triangle.y];
        auto p3 = points[triangle.z];
        auto normal = glm::normalize(glm::cross((p3 - p1), (p2 - p1)));

        out_points.push_back(p1);
        out_normals.push_back(normal);
        out_points.push_back(p2);
        out_normals.push_back(normal);
        out_points.push_back(p3);
        out_normals.push_back(normal);
        out_triangles.push_back(glm::uvec3(vertex_index++, vertex_index++, vertex_index++));
    }
}

inline void mesh_compute_smooth_normals(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& indices, std::vector<glm::vec3>& normals)
{
    normals.resize(points.size());
    for(uint32_t i = 0; i < indices.size(); ++i)
    {
        const glm::uvec3& face = indices[i];
        const glm::vec3& p1 = points[face.x];
        const glm::vec3& p2 = points[face.y];
        const glm::vec3& p3 = points[face.z];
        const glm::vec3 normal = glm::cross(p2 - p1, p3 - p1);

        const float angle_1 = glm::angle(glm::normalize(p2 - p1), glm::normalize(p3 - p1));
        const float angle_2 = glm::angle(glm::normalize(p3 - p2), glm::normalize(p1 - p2));
        const float angle_3 = glm::angle(glm::normalize(p1 - p3), glm::normalize(p2 - p3));

        normals[face.x] += normal * angle_1;
        normals[face.y] += normal * angle_2;
        normals[face.z] += normal * angle_3;
    }

    for(int i = 0; i < normals.size(); ++i)
        normals[i] = glm::normalize(normals[i]);
}

inline SoA_Region_List SoA_mesh_from_position_const_color4_with_flat_normal(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles, glm::vec4 color, std::vector<float>& out_data, std::vector<glm::uvec3>& out_triangles)
{
    std::vector<glm::vec3> _points, _normals;
    mesh_convert_to_flat_normals(points, triangles, _points, _normals, out_triangles);

    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(_points[0]), _points.size() }, SoA_Memcpy_Descriptor::ARRAY,  _points.data() },
        { { "color", sizeof(color), _points.size() }, SoA_Memcpy_Descriptor::CONSTANT, &color },
        { { "normal", sizeof(_normals[0]), _normals.size() }, SoA_Memcpy_Descriptor::ARRAY, _normals.data() },
    };

    ASSERT_MSG_DEBUG(SoA_get_size_bytes(descriptors) % sizeof(float) == 0, "Remainder was not 0.");
    out_data.resize(SoA_get_size_bytes(descriptors) / sizeof(float));
    SoA_memcpy(out_data.data(), descriptors);

    return SoA_create_region_list(descriptors);
}

inline SoA_Region_List SoA_mesh_from_position_normals_const_color4(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& normals, const std::vector<glm::uvec3>& triangles, glm::vec4 color, std::vector<float>& out_data)
{
    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(points[0]), points.size() }, SoA_Memcpy_Descriptor::ARRAY,  (void*)points.data() },
        { { "color", sizeof(color), points.size() }, SoA_Memcpy_Descriptor::CONSTANT, &color },
        { { "normal", sizeof(normals[0]), normals.size() }, SoA_Memcpy_Descriptor::ARRAY, (void*)normals.data() },
    };

    ASSERT_MSG_DEBUG(SoA_get_size_bytes(descriptors) % sizeof(float) == 0, "Remainder was not 0.");
    out_data.resize(SoA_get_size_bytes(descriptors) / sizeof(float));
    SoA_memcpy(out_data.data(), descriptors);

    return SoA_create_region_list(descriptors);
}

inline SoA_Region_List SoA_mesh_from_position_with_flat_normal(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles, std::vector<float>& out_data, std::vector<glm::uvec3>& out_triangles)
{
    std::vector<glm::vec3> _points, _normals;
    mesh_convert_to_flat_normals(points, triangles, _points, _normals, out_triangles);

    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(_points[0]), _points.size() }, SoA_Memcpy_Descriptor::ARRAY,  _points.data() },
        { { "normal", sizeof(_normals[0]), _normals.size() }, SoA_Memcpy_Descriptor::ARRAY, _normals.data() },
    };

    ASSERT_MSG_DEBUG(SoA_get_size_bytes(descriptors) % sizeof(float) == 0, "Remainder was not 0.");
    out_data.resize(SoA_get_size_bytes(descriptors) / sizeof(float));
    SoA_memcpy(out_data.data(), descriptors);

    return SoA_create_region_list(descriptors);
}

#endif // UTIL_MESH_H