#include <openvdb/openvdb.h>

#include "Resource.hpp"

class MeshResource;
class NrrdResource;

#ifndef OPENVDB_RESOURCE_H
#define OPENVDB_RESOURCE_H

class OpenvdbResource : public ResourceCRTP<OpenvdbResource>
{
    public:
        OpenvdbResource(const boost::filesystem::path& openvdb_path);
        OpenvdbResource(const NrrdResource& nrrd);
        OpenvdbResource(const MeshResource& mesh);

        static std::string getFileExtension();
        static boost::filesystem::path getPrefixDirectory();

        int cache(openvdb::FloatGrid::Ptr* output_openvdb_grid = nullptr, bool force_cache = false) const;
        int load(openvdb::FloatGrid::Ptr* grid) const;
        std::string getGridName() const;
};

#endif