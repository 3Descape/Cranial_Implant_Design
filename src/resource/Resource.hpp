#include <string>
#include <type_traits>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#ifndef RESOURCE_H
#define RESOURCE_H

class Resource
{
    public:
        typedef enum Type
        {
            DATA, // unique data stored somewhere
            CACHED // the data is genrated from another CACHE or DATA resource
        } Type;

        Resource(const boost::filesystem::path& file_path) : file_path_(file_path) {}

        static int setDataRootDirectory(const boost::filesystem::path& root_path);
        static boost::filesystem::path getDataRootDirectory();
        static int setCacheRootDirectory(const boost::filesystem::path& cache_root_dir);
        static boost::filesystem::path getCacheRootDirectory() { return cache_root_dir_ ? *cache_root_dir_ : getDataRootDirectory(); }
        static void setCachePrefixDirectory(const boost::filesystem::path& cache_prefix_dir) { cache_prefix_dir_ = cache_prefix_dir; }
        static boost::filesystem::path getCachePrefixDirectory() { return cache_prefix_dir_; }
        static boost::filesystem::path getCacheDirectory() { return getCacheRootDirectory() / getCachePrefixDirectory(); }
        static int createDirectoryIfNecessary(const boost::filesystem::path& directory);

        boost::filesystem::path getFileName() const { return file_path_.stem(); }
        boost::filesystem::path getRelativeFileDirectory() const { return file_path_.parent_path(); }
        boost::filesystem::path getRelativeFilePath() const { return file_path_; }

    protected:
        static boost::optional<boost::filesystem::path> data_root_dir_;
        static boost::optional<boost::filesystem::path> cache_root_dir_;
        static boost::filesystem::path cache_prefix_dir_;

        boost::filesystem::path file_path_;
};

template<typename CRTP>
class ResourceCRTP : public Resource
{
    using Resource::Resource;

    protected:
        static Type type_;

    public:

        ResourceCRTP(const boost::filesystem::path& file_path) : Resource(file_path) {}

        template <typename T>
        static boost::filesystem::path createFilePathFromOtherResource(const T& resource)
        {
            //static_assert(std::is_base_of<ResourceCRTP, T>::value, "T must inherit from ResourceCRTP");
            return resource.getRelativeFilePath().replace_extension(CRTP::getFileExtension());
        }

        template <typename T>
        T createInstanceFromResource()
        {
            return T(*this);
        }

        static boost::filesystem::path getRootDirectory()
        {
            if(type_ == Resource::CACHED)
                return getCacheDirectory() / CRTP::getPrefixDirectory();

            return getDataRootDirectory() / CRTP::getPrefixDirectory();
        }

        static boost::filesystem::path getRelativePath(boost::filesystem::path path)
        {
            return boost::filesystem::relative(path, getRootDirectory());
        }

        boost::filesystem::path getPrefixedAbsoluteFilePath(const boost::filesystem::path& prefix) const
        {
            return getRootDirectory() / prefix / getRelativeFilePath();
        }

        boost::filesystem::path getAbsoluteFilePath() const
        {
            return getRootDirectory() / getRelativeFilePath();
        }

        bool exists() const
        {
            return boost::filesystem::exists(getAbsoluteFilePath());
        }
};

#endif