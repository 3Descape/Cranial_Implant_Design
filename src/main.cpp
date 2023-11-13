#include <openvdb/openvdb.h>

#include "Application.hpp"
#include "resource/Resource.hpp"

int cache();

int main(int argc, char const *argv[])
{
    openvdb::initialize();

    if(Resource::setDataRootDirectory())
        return -1;

    std::cout << "Data root directory set to " << Resource::getDataRootDirectory().string() << std::endl;

    if(app.init())
        return -1;

    if(!Resource::getDataRootDirectory().empty())
        cache();

    app.run();

    return 0;
}