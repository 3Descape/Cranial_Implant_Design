#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

#include "Resource.hpp"
#include "MeshResource.hpp"
#include "util/util_timer.hpp"
#include "logger/logger.hpp"
#include "assert.hpp"

class NrrdResource;

#pragma once

class DescriptorResource : public ResourceCRTP<DescriptorResource>
{
    public:
        DescriptorResource(const boost::filesystem::path& file_path);
        DescriptorResource(const MeshResource& mesh_resource);
        DescriptorResource(const NrrdResource& nrrd_resource);

        static std::string getFileExtension();
        static boost::filesystem::path getPrefixDirectory();

        template<typename Descriptor_T>
        int cache(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& normals, bool force_cache = false) const;
        template<typename Descriptor_T>
        int load(Descriptor_T& descriptor) const;
        template<typename Descriptor_T>
        static int computeDescriptor(Descriptor_T& descriptor, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& normals);
        template<typename Descriptor_T>
        static int findMatches(const Descriptor_T& descriptor, uint32_t count, std::vector<MeshResource>& matches);
};

template<typename Descriptor_T>
int DescriptorResource::cache(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& normals, bool force_cache) const
{
    if(!force_cache && exists())
        return 0;

    const boost::filesystem::path file_path = getPrefixedAbsoluteFilePath(Descriptor_T::name);
    if(int code = createDirectoryIfNecessary(file_path.parent_path())) return code;

    Timer timer;
    LOG_INFO("Start caching descriptor file {}", file_path.string());

    Descriptor_T descriptor;
    if(int code = computeDescriptor(descriptor, points, normals)) return code;

    std::ofstream out_file;
    out_file.open(file_path.string(), std::ios::out | std::ios::binary | std::ios::trunc);
    if(!out_file.is_open()) return -1;

    out_file.write((char*)descriptor.data(), Descriptor_T::size);
    out_file.close();

    LOG_INFO("Cached descriptor file {} in {}", file_path.string(), timer.stop().toString());

    return 0;
}

template<typename Descriptor_T>
int DescriptorResource::load(Descriptor_T& descriptor) const
{
    const boost::filesystem::path file_path = getPrefixedAbsoluteFilePath(Descriptor_T::name);
    if(!boost::filesystem::exists(file_path))
        return -1;

    Timer timer;

    std::ifstream in_file;
    in_file.open(file_path.string(), std::ios::in | std::ios::binary | std::ios::ate);

    if(!in_file.is_open()) return -1;

    std::streampos size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);
    ASSERT(Descriptor_T::size == size);
    in_file.read((char*)descriptor.data(), Descriptor_T::size);
    in_file.close();

    timer.stop();
    LOG_INFO("Reading descriptor {} took {}.", file_path.string(), timer.toString());

    return 0;
}

template<typename Descriptor_T>
int DescriptorResource::computeDescriptor(Descriptor_T& descriptor, const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& normals)
{
    int code = 0;
    if constexpr(descriptor.needsNormals())
        code = descriptor.compute(points, normals);
    else
        code = descriptor.compute(points);

    return code;
}

template<typename A, typename B>
bool sortByOperator(const std::pair<A, B>& a, const std::pair<A, B>& b)
{
    return a.first < b.first;
}

template<typename Descriptor_T>
int DescriptorResource::findMatches(const Descriptor_T& descriptor, uint32_t count, std::vector<MeshResource>& matches)
{
    std::vector<std::pair<double, DescriptorResource>> distances;
    for(boost::filesystem::recursive_directory_iterator itr(DescriptorResource::getRootDirectory()); itr != boost::filesystem::recursive_directory_iterator{}; ++itr) {
        // skip directories itself
        if(!boost::filesystem::is_regular_file(itr->path()) || boost::filesystem::extension(itr->path()) != getFileExtension()) {
            continue;
        }

        DescriptorResource descriptor_resource(boost::filesystem::relative(itr->path(), getRootDirectory() / Descriptor_T::name));
        Descriptor_T source_descriptor;
        descriptor_resource.load(source_descriptor);

        const double distance = descriptor.distance(source_descriptor);
        distances.push_back({distance, descriptor_resource});
    }

    std::sort(distances.begin(), distances.end(), sortByOperator<double, DescriptorResource>);

    for(auto& pair : distances) {
        LOG_INFO("{}: ", pair.first, pair.second.getAbsoluteFilePath().string());
    }

    matches.reserve(count);
    for(uint32_t i = 0; i < count; ++i) {
        matches.push_back(MeshResource(distances[i].second));
    }

    return 0;
}