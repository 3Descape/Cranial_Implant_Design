#include <fstream>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <pcl/point_types.h>


#include "DescriptorResource.hpp"
#include "MeshResource.hpp"
#include "NrrdResource.hpp"
#include "util/util_timer.hpp"

DescriptorResource::Type DescriptorResource::type_ = DescriptorResource::CACHED;

DescriptorResource::DescriptorResource(const boost::filesystem::path& file_path) : ResourceCRTP(file_path)
{
}

DescriptorResource::DescriptorResource(const MeshResource& mesh_resource) : ResourceCRTP(createFilePathFromOtherResource(mesh_resource))
{
}

DescriptorResource::DescriptorResource(const NrrdResource& nrrd_resource) : ResourceCRTP(createFilePathFromOtherResource(nrrd_resource))
{
}

std::string DescriptorResource::getFileExtension()
{
    return ".bin";
}

boost::filesystem::path DescriptorResource::getPrefixDirectory()
{
    return boost::filesystem::path("descriptor");
}