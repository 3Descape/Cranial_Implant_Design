#include <boost/filesystem.hpp>
#include "resource/NrrdResource.hpp"
#include "resource/MeshResource.hpp"

int cache()
{
    for(boost::filesystem::recursive_directory_iterator itr(NrrdResource::getRootDirectory()); itr != boost::filesystem::recursive_directory_iterator{}; ++itr)
    {
        // skip directories itself
        if (!boost::filesystem::is_regular_file(itr->path()))
        {
            // don't recurse into the cache directory
            if (itr->path() == Resource::getCacheDirectory())
                itr.no_push();

            continue;
        }

        NrrdResource nrrd_resource(NrrdResource::getRelativePath(itr->path()));
        MeshResource mesh_resource(nrrd_resource);

        if(int code = mesh_resource.cache())
            return code;
    }

    return 0;
}