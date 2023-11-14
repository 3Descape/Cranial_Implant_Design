#include <openvdb/openvdb.h>
#include <memory>

#include "application.hpp"
#include "logger/logger.hpp"
#include "logger/stdout_logger.hpp"
#include "resource/Resource.hpp"

int cache();

std::shared_ptr<LoggerInterface> logger = std::make_shared<StdoutLogger>();

int main(int argc, char const *argv[])
{
    openvdb::initialize();

    if(Resource::setDataRootDirectory())
        return -1;

    LOG_INFO("Data root directory set to {}", Resource::getDataRootDirectory().string());

    if(app.init())
        return -1;

    if(!Resource::getDataRootDirectory().empty())
        cache();

    app.run();

    return 0;
}