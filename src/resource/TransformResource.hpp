#include "NrrdResource.hpp"
#include <openvdb/openvdb.h>

class NrrdResource;

#ifndef TRANSFORM_RESOURCE_H
#define TRANSFORM_RESOURCE_H

class TransformResource : public ResourceCRTP<TransformResource>
{
    public:
        TransformResource(const NrrdResource& nrrd_resource);

        static std::string getFileExtension();
        static boost::filesystem::path getPrefixDirectory();

        int write(const openvdb::Vec3d& transformation);
        openvdb::Vec3d TransformResource::read();
};

#endif