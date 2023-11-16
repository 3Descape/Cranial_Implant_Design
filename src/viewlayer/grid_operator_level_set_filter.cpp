#include <imgui/imgui.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetFilter.h>

#include "logger/logger.hpp"
#include "grid_operator_level_set_filter.hpp"

GridOperatorLevelSetFilter::GridOperatorLevelSetFilter()
    : GridOperator(GridOperator_LevelSetFilter) {}

void GridOperatorLevelSetFilter::execute() {
    out_grids.clear();
    for(auto& grid_ptr : *in_grids) {
        openvdb::FloatGrid::Ptr out_grid = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(grid_ptr);
        openvdb::tools::LevelSetFilter<openvdb::FloatGrid> filter(*out_grid);
        if(level_set_filter_type == LevelSetFilterType_Laplacian) {
            for(int i = 0; i < iteration_count; ++i)
                filter.laplacian();
        }
        else if(level_set_filter_type == LevelSetFilterType_Gaussian) {
            for(int i = 0; i < iteration_count; ++i)
                filter.gaussian(width);
        }
        else if(level_set_filter_type == LevelSetFilterType_Median)
            filter.median(width);
        else if(level_set_filter_type == LevelSetFilterType_Mean)
            filter.mean(width);
        else if(level_set_filter_type == LevelSetFilterType_MeanCurvature)
            filter.meanCurvature();
        else {
            LOG_ERROR("GridOperator LevelSetFilter unsupported type.");
            exit(-1);
        }
        out_grids.push_back(out_grid);
    }
}

void GridOperatorLevelSetFilter::drawUI() {
    ImGui::Checkbox("Enabled", &enabled);
    if(enabled && has_data) {
        ImGui::Text("Input Grids: %d, Output Grids: %d", in_grids->size(), out_grids.size());
    }
    ImGui::BeginDisabled(!enabled);
    if(ImGui::BeginCombo("##filter_type", level_set_filter_type_names[level_set_filter_type])) {
        for(int32_t i = 0; i < LevelSetFilterType_MAX; ++i) {
            bool is_selected = level_set_filter_type == i;
            if(ImGui::Selectable(level_set_filter_type_names[i], is_selected)) {
                level_set_filter_type = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::DragInt("Iterations", &iteration_count, 1, 1, 10);
    if(level_set_filter_type == LevelSetFilterType_Gaussian ||
        level_set_filter_type == LevelSetFilterType_Median ||
        level_set_filter_type == LevelSetFilterType_Mean
    ) {
        ImGui::DragInt("Width", &width, 1, 1, 10);
    }
    ImGui::EndDisabled();
}