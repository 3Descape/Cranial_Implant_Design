#include "NrrdResource.hpp"
#include "OpenvdbResource.hpp"

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
    auto nrrd_file = getAbsoluteFilePath();
    auto start = std::chrono::steady_clock::now();
    std::cout << "Start loading nrrd file " << nrrd_file.string() << std::endl;
    assert(nrrd_file.extension() == getFileExtension());
    destination = NRRD::Image<float>(nrrd_file.string());
    if (!destination)
    {
        std::cout << "Failed to read NRRD image \"" << nrrd_file.string() << "\"" << std::endl;
        return -1;
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "End loading nrrd file. Took: " << float(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.f << " seconds." << std::endl;

    return 0;
}