#include <chrono>
#include <thread>
#include "math.h"
#include <stdexcept>
#include <fstream>

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/TopologyToLevelSet.h>
#include <openvdb/tools/Composite.h>

#include "OpenvdbResource.hpp"
#include "NrrdResource.hpp"
#include "MeshResource.hpp"
#include "TransformResource.hpp"
#include "util/util_timer.hpp"
#include "util/util_path.hpp"
#include "logger/logger.hpp"
#include "assert.hpp"

OpenvdbResource::Type OpenvdbResource::type_ = OpenvdbResource::CACHED;

OpenvdbResource::OpenvdbResource(const boost::filesystem::path& openvdb_path) : ResourceCRTP(openvdb_path)
{
}

OpenvdbResource::OpenvdbResource(const NrrdResource& nrrd) : ResourceCRTP(createFilePathFromOtherResource(nrrd))
{
}

OpenvdbResource::OpenvdbResource(const MeshResource& mesh) : ResourceCRTP(createFilePathFromOtherResource(mesh))
{
}

std::string OpenvdbResource::getFileExtension()
{
    return ".vdb";
}

boost::filesystem::path OpenvdbResource::getPrefixDirectory()
{
    return boost::filesystem::path("openvdb");
}

int OpenvdbResource::cache(openvdb::FloatGrid::Ptr* output_openvdb_grid, bool force_cache) const
{
    if(!force_cache && boost::filesystem::exists(getAbsoluteFilePath()))
        return 0;

    Timer timer;
    LOG_INFO("Start caching openvdb file {}", getAbsoluteFilePath().string());

    NrrdResource nrrd_resource(*this);
    NRRD::Image<float> image;
    if(int code = nrrd_resource.load(image)) return code;

    ASSERT_MSG(image.dimension() == 3, "NRRD image has not dimension == 3.");

    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(1.0);
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    const openvdb::FloatGrid::ValueType inside = -grid->background();

    for(int i = 0; i < image.size(0); ++i)
    {
        for(int j = 0; j < image.size(1); ++j)
        {
            for(int k = 0; k < image.size(0); ++k)
            {
                const float val = image.pixel(i, j, k);
                if(val == 0) continue;
                accessor.setValue(openvdb::Coord(i, j, k), inside);
            }
        }
    }

    grid = openvdb::tools::topologyToLevelSet<openvdb::FloatGrid>(*grid, 3, 1, 0, 2);
    grid->setGridClass(openvdb::GRID_LEVEL_SET);
    grid->setName(getGridName());

    std::vector<std::string> path_fragments = path_split(nrrd_resource.getRelativeFilePath());
    openvdb::math::Transform::Ptr linearTransform = openvdb::math::Transform::createLinearTransform(1.0);
    openvdb::Vec3d translation;
    if(path_fragments[0] == "implant") {
        NrrdResource transformation_source(path_replace_root(nrrd_resource.getRelativeFilePath(), "defective_skull"));
        TransformResource transformation_resource(transformation_source);
        translation = transformation_resource.read();
    } else {
        openvdb::CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
        openvdb::Coord dim = bbox.dim();
        translation = -bbox.getCenter() + openvdb::Vec3d(0, 0, dim.z() / 2.0f);
        TransformResource transform_resource(nrrd_resource);
        transform_resource.write(translation);
    }
    linearTransform->postTranslate(translation);
    linearTransform->postScale(0.1);
    grid->setTransform(linearTransform);

    const auto file_path = getAbsoluteFilePath();
    if(int code = createDirectoryIfNecessary(file_path.parent_path())) return code;
    openvdb::io::File file(file_path.string());
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids_container;
    grids_container.push_back(grid);
    // Write out the contents of the container.
    file.write(grids_container);
    file.close();

    LOG_INFO("Cached OpenVDB file in {}", timer.stop().toString());

    if(output_openvdb_grid != nullptr)
        *output_openvdb_grid = grid;

    return 0;
}

int OpenvdbResource::load(openvdb::FloatGrid::Ptr* grid) const
{
    if(!exists())
        return cache(grid);

    LOG_INFO("Loading {}", getAbsoluteFilePath().string());

    try
    {
        openvdb::io::File file(getAbsoluteFilePath().string());

        file.open();

        if(file.hasGrid(getGridName()))
        {
            openvdb::GridBase::Ptr baseGrid = file.readGrid(getGridName());
            *grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
            file.close();
        }
        else if(file.beginName() != file.endName()) // fallback, if there is at least one grid, take the first
        {
            for (openvdb::io::File::NameIterator iter(file.beginName()); iter != file.endName(); ++iter) {
                LOG_INFO("Grid: {}", *iter);
            }
            openvdb::GridBase::Ptr baseGrid = file.readGrid(*file.beginName());
            baseGrid->setName(getGridName());
            *grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
            file.close();
            // Add the grid pointer to a container.
            openvdb::GridPtrVec grids_container;
            grids_container.push_back(*grid);
            // Write out the contents of the container.
            file.write(grids_container);
        }
        else
        {
            file.close();
            return cache(grid, true);
        }

        return 0;
    }
    catch (const std::length_error& e)
    {
        LOG_ERROR("{} : This most likely happened because using a release build of OpenVDB in debug mode!!!", e.what());
        exit(-1);
    }
    catch(const std::exception& e)
    {
        LOG_WARN("Openvdb read grid exception occured: {}", e.what());
        return cache(grid, true);
    }

    return 0;
}

std::string OpenvdbResource::getGridName() const
{
    return getRelativeFilePath().string();
}