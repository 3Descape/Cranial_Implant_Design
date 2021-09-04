#pragma once
#include "grid_operator.hpp"

enum FilterType {
    FilterType_ActiveVoxelCount_LessThan = 0,
    FilterType_ActiveVoxelCount_GreaterThan = 1,
    FilterType_TakeCount = 2,
    FilterType_MAX = 3,
};

inline const char* filter_type_names[] = {
    "Active Voxel Count Less Than",
    "Active Voxel Count Greater Than",
    "Take Count",
};

struct GridOperatorFilter: public GridOperator {
    GridOperatorFilter();
    virtual void execute() override;
    virtual void drawUI() override;

    int filter_type = FilterType_TakeCount;
    int threshold_active_voxel_count = 1000;
    int take_count = 1;
    int take_count_offset = 0;
};