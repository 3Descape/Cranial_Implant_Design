#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#ifndef UTIL_PATH_H
#define UTIL_PATH_H

std::vector<std::string> path_split(const boost::filesystem::path& path) {
      std::vector<std::string> fragments;
      for(auto& fragment : path) {
        fragments.push_back(fragment.string());
      }

      return fragments;
}

boost::filesystem::path path_replace_root(const boost::filesystem::path& path, const boost::filesystem::path& root_replacement) {
      std::vector<std::string> path_fragments = path_split(path);

      boost::filesystem::path new_path = root_replacement;
      for(int i = 1; i < path_fragments.size(); ++i) {
        new_path /= path_fragments[i];
      }

      return new_path;
}
#endif