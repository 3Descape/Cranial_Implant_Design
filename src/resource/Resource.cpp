#include "Resource.hpp"
#include "logger/logger.hpp"


std::optional<boost::filesystem::path> Resource::data_root_dir_;
std::optional<boost::filesystem::path> Resource::cache_root_dir_;
boost::filesystem::path Resource::cache_prefix_dir_("cache");

int Resource::setDataRootDirectory(std::optional<std::string> new_path)
{
    if(new_path) {
        if(!boost::filesystem::exists(*new_path)) {
            return -1;
        }

        std::ofstream settings("./settings.txt", std::ios::trunc);
        if(!settings) {
            return -1;
        }
        settings << *new_path;
        settings.close();

        data_root_dir_ = boost::filesystem::path(*new_path);

        return 0;
    }

    if(!boost::filesystem::exists("./settings.txt")) {
        return 0;
    }

    std::ifstream settings("./settings.txt");
    if(!settings) {
        return -1;
    }

    std::string data_root_dir_in;
    std::getline(settings, data_root_dir_in);
    settings.close();

    if(!boost::filesystem::exists(data_root_dir_in)) {
        LOG_WARN("Data root directory {} does not excist.", data_root_dir_in);
        return -1;
    }

    data_root_dir_ = data_root_dir_in;

    return 0;
}

int Resource::setCacheRootDirectory(const boost::filesystem::path& cache_root_dir)
{
  if (!boost::filesystem::exists(cache_root_dir)) {
    LOG_WARN("Root directory {} does not exist.", cache_root_dir.string());
    return -1;
  }

  cache_root_dir_ = cache_root_dir;

  return 0;
}

boost::filesystem::path Resource::getDataRootDirectory()
{
    if(!data_root_dir_) {
        return boost::filesystem::path();
    }

    return *data_root_dir_;
}

int Resource::createDirectoryIfNecessary(const boost::filesystem::path& directory)
{
    if(!boost::filesystem::exists(directory)) {
        if(!boost::filesystem::create_directories(directory)) {
            LOG_ERROR("Failed to create directory {}", directory.string());
            return -1;
        }
    }

    return 0;
}