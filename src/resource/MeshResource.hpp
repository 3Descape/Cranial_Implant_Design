#include <openvdb/openvdb.h>
#include <glm/glm.hpp>

#include "Resource.hpp"

class NrrdResource;
class DescriptorResource;

#ifndef MESH_RESOURCE_H
#define MESH_RESOURCE_H

class MeshResource : public ResourceCRTP<MeshResource>
{
    private:
        inline static double adaptivity_ = 0.5f;

    public:
        MeshResource(const boost::filesystem::path& mesh_path);
        MeshResource(const NrrdResource& nrrd);
        MeshResource(const DescriptorResource& nrrd);

        static std::string getFileExtension();
        static boost::filesystem::path getPrefixDirectory();

        int cache(const openvdb::FloatGrid::Ptr* input_openvdb_grid = nullptr, bool force_cache = false) const;
        int loadWithCaching(std::vector<glm::vec3>& points_out, std::vector<glm::uvec3>& triangles_out) const;
        int load(std::vector<glm::vec3>* points_out, std::vector<glm::uvec3>* triangles_out) const;
        static int gridToMeshData(const openvdb::FloatGrid::Ptr grid, std::vector<glm::vec3>& points, std::vector<glm::uvec3>& indices);
};

#endif