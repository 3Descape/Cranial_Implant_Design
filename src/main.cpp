#include <openvdb/openvdb.h>

#include "Application.hpp"
#include "resource/Resource.hpp"

int cache();

int main(int argc, char const *argv[])
{
    openvdb::initialize();

    if(Resource::setDataRootDirectory("C:\\Users\\micla\\Desktop\\c++\\data"))
        return -1;

    if(app.init())
        return -1;

    cache();

    app.run();

    return 0;
}