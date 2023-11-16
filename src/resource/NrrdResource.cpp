#include "NrrdResource.hpp"
#include "OpenvdbResource.hpp"
#include "logger/logger.hpp"
#include "util/util_timer.hpp"
#include "assert.hpp"

NrrdResource::Type NrrdResource::type_ = NrrdResource::DATA;

NrrdResource::NrrdResource(const boost::filesystem::path& nrrd_path) : ResourceCRTP(nrrd_path)
{
}

NrrdResource::NrrdResource(const OpenvdbResource& openvdb) : ResourceCRTP(createFilePathFromOtherResource(openvdb))
{
}

std::string NrrdResource::getFileExtension()
{
    return ".nrrd";
}

boost::filesystem::path NrrdResource::getPrefixDirectory()
{
    return boost::filesystem::path("nrrd");
}

int NrrdResource::load(NRRD::Image<float>& destination) const
{
    boost::filesystem::path nrrd_file = getAbsoluteFilePath();
    Timer timer;
    LOG_INFO("Start loading nrrd file {}", nrrd_file.string());
    ASSERT(nrrd_file.extension() == getFileExtension());
    destination = NRRD::Image<float>(nrrd_file.string());
    if (!destination) {
        LOG_ERROR("Failed to read NRRD image {}", nrrd_file.string());
        return -1;
    }

    LOG_INFO("End loading nrrd file. Took {}", timer.stop().toString());

    return 0;
}