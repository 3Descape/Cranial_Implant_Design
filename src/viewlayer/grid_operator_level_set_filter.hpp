#pragma once
#include "grid_operator.hpp"

enum LevelSetFilterType {
    LevelSetFilterType_Laplacian = 0,
    LevelSetFilterType_Gaussian = 1,
    LevelSetFilterType_Median = 2,
    LevelSetFilterType_Mean = 3,
    LevelSetFilterType_MeanCurvature = 4,
    LevelSetFilterType_MAX = 5,
};

inline const char* level_set_filter_type_names[] = {
    "Laplacian",
    "Gaussian",
    "Median",
    "Mean",
    "MeanCurvature",
};

struct GridOperatorLevelSetFilter: public GridOperator {
    GridOperatorLevelSetFilter();
    virtual void execute() override;
    virtual void drawUI() override;

    int iteration_count = 1;
    int width = 1;
    int level_set_filter_type = LevelSetFilterType_Laplacian;
};