#include <NRRD/nrrd_image.hxx>
#include "Resource.hpp"

class OpenvdbResource;

#ifndef NRRD_RESOURCE_H
#define NRRD_RESOURCE_H

class NrrdResource : public ResourceCRTP<NrrdResource>
{
    public:
        NrrdResource(const boost::filesystem::path& nrrd_path);
        NrrdResource(const OpenvdbResource& openvdb);

        static std::string getFileExtension();
        static boost::filesystem::path getPrefixDirectory();

        int load(NRRD::Image<float>& destination) const;
};

#endif