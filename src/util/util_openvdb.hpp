#include <map>

#include <openvdb/openvdb.h>
#include <openvdb/tools/GridOperators.h>
#include <glm/gtx/euler_angles.hpp>
#include "scene_object/SceneObject.hpp"
#include "SoA.hpp"
#include "util/util_timer.hpp"

#ifndef UTIL_OPENVDB
#define UTIL_OPENVDB
inline void glm_to_openvdb_transform(const SceneObject::Transformation& transformation, openvdb::math::Transform& linearTransform)
{
    glm::dvec3 rotation_radians = glm::radians(transformation.rotation);
    openvdb::math::Mat4d old_transform = linearTransform.baseMap()->getAffineMap()->getMat4();

    old_transform.postScale(openvdb::math::Vec3d(transformation.scale.x, transformation.scale.y, transformation.scale.z));
    old_transform.postRotate(openvdb::math::Z_AXIS, rotation_radians.z);
    old_transform.postRotate(openvdb::math::X_AXIS, rotation_radians.x);
    old_transform.postRotate(openvdb::math::Y_AXIS, rotation_radians.y);
    old_transform.postTranslate(openvdb::math::Vec3d(transformation.translation.x, transformation.translation.y, transformation.translation.z));

    linearTransform = *openvdb::math::Transform::createLinearTransform(old_transform);
}

inline SoA_Region_List openvdb_create_node_tree_wireframe_visualization(const openvdb::FloatGrid& grid, std::vector<float>& out_data, std::vector<glm::uvec2>& indices)
{
    Timer timer;
    openvdb::Index64 nodeCount = grid.tree().activeLeafVoxelCount() + grid.tree().nonLeafCount();
    const openvdb::Index64 N = nodeCount * 8;

    std::vector<glm::vec3> points;
    std::vector<GLubyte> levels;
    points.reserve(N);
    levels.reserve(N);
    indices.reserve(N);

    openvdb::CoordBBox bbox;
    openvdb::Index64 pOffset = 0;

    std::map<openvdb::Vec3d, openvdb::Index64> point_lookup;
    for(openvdb::FloatGrid::TreeType::NodeCIter iter = grid.tree().cbeginNode(); iter; ++iter)
    {
        iter.getBoundingBox(bbox);

        // Nodes are rendered as cell-centered
        // Coordinates are in index space
        const openvdb::Vec3d min(bbox.min().x()-0.5, bbox.min().y()-0.5, bbox.min().z()-0.5);
        const openvdb::Vec3d max(bbox.max().x()+0.5, bbox.max().y()+0.5, bbox.max().z()+0.5);

        openvdb::Vec3d point_coords[8] = {
            min,
            {min.x(), min.y(), max.z()},
            {max.x(), min.y(), max.z()},
            {max.x(), min.y(), min.z()},
            {min.x(), max.y(), min.z()},
            {min.x(), max.y(), max.z()},
            max,
            {max.x(), max.y(), min.z()},
        };
        openvdb::Index64 point_indexes[8];

        for(int i = 0; i < 8; ++i)
        {
            const openvdb::Vec3d point = grid.indexToWorld(point_coords[i]);
            auto cache = point_lookup.find(point);
            if(cache != point_lookup.end())
                point_indexes[i] = cache->second;
            else
            {
                points.emplace_back(point[0], point[1], point[2]);
                point_indexes[i] = pOffset++;
            }
        }

        for(int i = 0; i < 8; ++i)
            levels.emplace_back(iter.getLevel());

        // edge 1
        indices.emplace_back(point_indexes[0], point_indexes[1]);
        // edge 2
        indices.emplace_back(point_indexes[1], point_indexes[2]);
        // edge 3
        indices.emplace_back(point_indexes[2], point_indexes[3]);
        // edge 4
        indices.emplace_back(point_indexes[3], point_indexes[0]);
        // edge 5
        indices.emplace_back(point_indexes[4], point_indexes[5]);
        // edge 6
        indices.emplace_back(point_indexes[5], point_indexes[6]);
        // edge 7
        indices.emplace_back(point_indexes[6], point_indexes[7]);
        // edge 8
        indices.emplace_back(point_indexes[7], point_indexes[4]);
        // edge 9
        indices.emplace_back(point_indexes[0], point_indexes[4]);
        // edge 10
        indices.emplace_back(point_indexes[1], point_indexes[5]);
        // edge 11
        indices.emplace_back(point_indexes[2], point_indexes[6]);
        // edge 12
        indices.emplace_back(point_indexes[3], point_indexes[7]);
    }

    std::cout << "N: " << points.size() << std::endl;

    std::vector<SoA_Memcpy_Descriptor> descriptors = {
        { { "position", sizeof(points[0]), points.size() }, SoA_Memcpy_Descriptor::ARRAY,  (void*)points.data() },
        { { "level", sizeof(levels[0]), levels.size() }, SoA_Memcpy_Descriptor::ARRAY, (void*)levels.data() },
    };

    assert(SoA_get_size_bytes(descriptors) % sizeof(float) == 0 && "Remainder was not 0.");
    out_data.resize(SoA_get_size_bytes(descriptors) / sizeof(float));
    SoA_memcpy(out_data.data(), descriptors);

    std::cout << "Create Grid Tree Wireframe took " << timer.stop().toString() << std::endl;

    return SoA_create_region_list(descriptors);
}

template<typename GridType>
void openvdb_extract_active_voxel_positions(const GridType& grid, std::vector<glm::vec3>& out_points) {
    openvdb::Vec3d ptn;
    size_t point_count = 0;
    out_points.resize(grid.activeVoxelCount());
    for(GridType::TreeType::ValueOnCIter iter = grid.tree().cbeginValueOn(); iter; ++iter, ++point_count)
    {
        if(!iter.isVoxelValue()) continue;

        openvdb::Coord coord = iter.getCoord();
        ptn = grid.indexToWorld(coord);

        out_points[point_count].x = static_cast<float>(ptn[0]);
        out_points[point_count].y = static_cast<float>(ptn[1]);
        out_points[point_count].z = static_cast<float>(ptn[2]);
    }
}

inline void openvdb_extract_active_voxel_values(const openvdb::FloatGrid& grid, std::vector<float>& out_values) {
    size_t value_count = 0;
    out_values.resize(grid.activeVoxelCount());
    for(openvdb::FloatGrid::TreeType::ValueOnCIter iter = grid.tree().cbeginValueOn(); iter; ++iter, ++value_count)
    {
        if(!iter.isVoxelValue()) continue;
        out_values[value_count] = *iter;
    }
}

inline void openvdb_extract_active_voxel_vector_length(const openvdb::Vec3SGrid& grid, std::vector<float>& out_values) {
    size_t value_count = 0;
    out_values.resize(grid.activeVoxelCount());
    for(openvdb::Vec3SGrid::TreeType::ValueOnCIter iter = grid.tree().cbeginValueOn(); iter; ++iter, ++value_count)
    {
        if(!iter.isVoxelValue()) continue;
        out_values[value_count] = (*iter).length();
    }
}

inline void openvdb_extract_active_voxel_normals(const openvdb::FloatGrid& grid, std::vector<glm::vec3>& out_normals) {
    openvdb::Vec3SGrid::Ptr normals_grid = openvdb::tools::gradient(grid);
    openvdb::Vec3d normal;
    size_t normal_count = 0;
    out_normals.resize(normals_grid->activeVoxelCount());
    for(openvdb::Vec3SGrid::TreeType::ValueOnCIter iter = normals_grid->tree().cbeginValueOn(); iter; ++iter, ++normal_count)
    {
        if(!iter.isVoxelValue()) continue;

        normal = iter.getValue();
        out_normals[normal_count].x = static_cast<float>(normal[0]);
        out_normals[normal_count].y = static_cast<float>(normal[1]);
        out_normals[normal_count].z = static_cast<float>(normal[2]);
    }
}

#endif // UTIL_OPENVDB