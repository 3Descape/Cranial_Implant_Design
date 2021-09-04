#pragma once

#include <vector>
#include <openvdb/openvdb.h>

enum GridOperatorType {
    GridOperator_SegmentSDF = 0,
    GridOperator_Filter = 1,
    GridOperator_LevelSetFilter = 2,
    // GridOperator_Border = 3,
    GridOperator_MAX = 3,
};

inline const char* grid_operator_type_names[] = {
    "Segment SDF",
    "Filter Grids",
    "Level Set Filter",
    // "Border",
};

struct GridOperator {
    int type;
    bool enabled = true;
    bool has_data = false;
    const std::vector<openvdb::FloatGrid::Ptr>* in_grids = nullptr;
    std::vector<openvdb::FloatGrid::Ptr> out_grids;

    GridOperator(int type);
    virtual void execute() = 0;
    virtual void drawUI() = 0;
};

std::vector<openvdb::FloatGrid::Ptr> grid_operator_execute_all(std::vector<GridOperator*>& grid_operations, const std::vector<openvdb::FloatGrid::Ptr>& in_grids);