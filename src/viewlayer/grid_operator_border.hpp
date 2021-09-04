#pragma once
#include "grid_operator.hpp"
#include "grid_operator_level_set_filter.hpp"

struct GridOperatorBorder: public GridOperator {
    GridOperatorBorder();
    virtual void execute() override;
    virtual void drawUI() override;

    int filter_type = LevelSetFilterType_Laplacian;
    bool use_mask = true;
    int iteration_count = 1;
    int width = 5;
    int mask_width = 10;
    float mask_max = 3.0f;
};