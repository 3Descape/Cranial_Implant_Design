#include "Resource.hpp"

#include <iostream>

boost::optional<boost::filesystem::path> Resource::data_root_dir_;
boost::optional<boost::filesystem::path> Resource::cache_root_dir_;
boost::filesystem::path Resource::cache_prefix_dir_("cache");

int Resource::setDataRootDirectory(const boost::filesystem::path& data_root_dir)
{
    if(!boost::filesystem::exists(data_root_dir))
    {
        std::cout << "Data root directory \"" << data_root_dir.string() << "\" does not exist." << std::endl;
        return -1;
    }

    data_root_dir_ = data_root_dir;

    return 0;
}

int Resource::setCacheRootDirectory(const boost::filesystem::path& cache_root_dir)
{
  if (!boost::filesystem::exists(cache_root_dir))
  {
    std::cout << "Root directory \"" << cache_root_dir.string() << "\" does not exist." << std::endl;
    return -1;
  }

  cache_root_dir_ = cache_root_dir;

  return 0;
}

boost::filesystem::path Resource::getDataRootDirectory()
{
    if(!data_root_dir_)
    {
        std::cout << "No root directory was set!" << std::endl;
        return boost::filesystem::path();
    }

    return *data_root_dir_;
}

int Resource::createDirectoryIfNecessary(const boost::filesystem::path& directory)
{
    if(!boost::filesystem::exists(directory))
    {
        if(!boost::filesystem::create_directories(directory))
        {
            std::cout << "Error creating directories: \"" << directory.string() << "\"" << std::endl;
            return -1;
        }
    }

    return 0;
}