#include <thread>

#include <tinyply.hpp>
#include <glm/glm.hpp>
#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <pcl/point_types.h>
#include <pcl/features/gasd.h>

#include "DescriptorResource.hpp"
#include "descriptor/GASDDescriptor.hpp"
#include "MeshResource.hpp"
#include "NrrdResource.hpp"
#include "OpenvdbResource.hpp"
#include "tinyply.hpp"
#include "util/util_mesh.hpp"
#include "util/util_timer.hpp"

MeshResource::Type MeshResource::type_ = MeshResource::CACHED;

MeshResource::MeshResource(const boost::filesystem::path& mesh_path) : ResourceCRTP(mesh_path)
{
}

MeshResource::MeshResource(const NrrdResource& nrrd_resource) : ResourceCRTP(createFilePathFromOtherResource(nrrd_resource))
{
}

MeshResource::MeshResource(const DescriptorResource& descriptor_resource) : ResourceCRTP(createFilePathFromOtherResource(descriptor_resource))
{
}

std::string MeshResource::getFileExtension()
{
    return ".ply";
}

boost::filesystem::path MeshResource::getPrefixDirectory()
{
    return boost::filesystem::path("mesh");
}

int MeshResource::cache(const openvdb::FloatGrid::Ptr* input_openvdb_grid, bool force_cache) const
{
    if(!force_cache && exists())
        return 0;

    std::cout << "Start caching mesh file \"" << getAbsoluteFilePath().string() << "\"" << std::endl;
    Timer timer;

    openvdb::FloatGrid::Ptr temporary_openvdb_grid = nullptr;
    if(!input_openvdb_grid)
    {
        OpenvdbResource openvdb_resource(*this);
        openvdb_resource.load(&temporary_openvdb_grid);
    }
    const openvdb::FloatGrid::Ptr openvdb_grid = input_openvdb_grid ? *input_openvdb_grid : temporary_openvdb_grid;

    std::vector<glm::vec3> mesh_points;
    std::vector<glm::uvec3> mesh_indices;

    gridToMeshData(openvdb_grid, mesh_points, mesh_indices);

    const auto file_path = getAbsoluteFilePath();
    if(int code = createDirectoryIfNecessary(file_path.parent_path())) return code;
    write_ply(file_path.string(), mesh_indices, mesh_points);

    std::cout << "Wrote cached mesh to \"" << getAbsoluteFilePath().string() << "\"" << std::endl;
    std::cout << "Generating mesh took " << timer.stop().toString() << std::endl;

    DescriptorResource describtor_resource(*this);
    std::vector<glm::vec3> mesh_normals;
    mesh_compute_smooth_normals(mesh_points, mesh_indices, mesh_normals);
    describtor_resource.cache<GASDDescriptor>(mesh_points, mesh_normals, force_cache);

    return 0;
}

int MeshResource::loadWithCaching(std::vector<glm::vec3>& points_out, std::vector<glm::uvec3>& triangles_out) const
{
    if(!exists())
        cache();

    return load(&points_out, &triangles_out);
}

int MeshResource::load(std::vector<glm::vec3>* points_out, std::vector<glm::uvec3>* triangles_out) const
{
    Timer timer;
    if(!exists()) return -1;
    if(int code = read_ply(getAbsoluteFilePath().string(), points_out, triangles_out)) return code;

    std::cout << "Reading mesh \"" << getAbsoluteFilePath().string() << "\" took " << timer.stop().toString() << "." << std::endl;

    return 0;
}

int MeshResource::gridToMeshData(const openvdb::FloatGrid::Ptr grid, std::vector<glm::vec3>& points, std::vector<glm::uvec3>& indices)
{
    std::vector<openvdb::Vec3s> openvdb_points;
    std::vector<openvdb::Vec3I> openvdb_triangles;
    std::vector<openvdb::Vec4I> openvdb_quats;
    openvdb::tools::volumeToMesh(*grid, openvdb_points, openvdb_triangles, openvdb_quats, 0.0, adaptivity_, true);

    openvdb_triangles.reserve(openvdb_triangles.size() + openvdb_quats.size() * 2);

    // triangulation of quats
    for(openvdb::Vec4I quats : openvdb_quats)
    {
        openvdb_triangles.push_back(openvdb::Vec3I(quats.x(), quats.y(), quats.z()));
        openvdb_triangles.push_back(openvdb::Vec3I(quats.x(), quats.z(), quats.w()));
    }

    points.reserve(openvdb_points.size());
    for(auto& point : openvdb_points)
        points.push_back(glm::vec3(point.x(), point.y(), point.z()));

    indices.reserve(openvdb_triangles.size());
    for(auto& triangle : openvdb_triangles)
        indices.push_back(glm::uvec3(triangle.x(), triangle.y(), triangle.z()));

    return 0;
}