#include <fstream>
#include <sstream>
#include "TransformResource.hpp"

TransformResource::Type TransformResource::type_ = Resource::CACHED;

TransformResource::TransformResource(const NrrdResource& nrrd_resource) : ResourceCRTP(createFilePathFromOtherResource(nrrd_resource))
{
}

std::string TransformResource::getFileExtension()
{
    return ".txt";
}

boost::filesystem::path TransformResource::getPrefixDirectory()
{
    return boost::filesystem::path("transformation");
}

int TransformResource::write(const openvdb::Vec3d& transformation)
{
    if(int code = createDirectoryIfNecessary(getAbsoluteFilePath().parent_path())) return code;
    std::ofstream file;
    file.open(getAbsoluteFilePath().string(), std::ios::out | std::ios::trunc);
    if(!file.is_open()) return -1;
    file << transformation.x() << " " << transformation.y() << " " << transformation.z() << std::endl;
    file.close();
    return 1;
}

openvdb::Vec3d TransformResource::read()
{
    std::ifstream file;
    file.open(getAbsoluteFilePath().string());
    if(!file.is_open()) return openvdb::Vec3d();
    std::string line;
    std::getline(file, line);
    std::istringstream in(line);
    double x, y, z;
    in >> x >> y >> z;
    file.close();
    return openvdb::Vec3d(x, y, z);
}