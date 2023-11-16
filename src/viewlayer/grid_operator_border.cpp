#include "grid_operator_border.hpp"
#include "imgui.h"

#include <openvdb/openvdb.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/Mask.h>

#include "application.hpp"
#include "logger/logger.hpp"

GridOperatorBorder::GridOperatorBorder()
    : GridOperator(GridOperator_Border) {}

void loadTargetGrid(openvdb::FloatGrid::Ptr& target_grid);

void GridOperatorBorder::execute() {
    out_grids.clear();

    openvdb::FloatGrid::Ptr target_grid;
    loadTargetGrid(target_grid);
    openvdb::FloatGrid::Ptr mask;
    if(use_mask) {
        mask = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(target_grid);
        openvdb::tools::LevelSetFilter<openvdb::FloatGrid> mask_filter(*mask);
        mask_filter.resize(mask_width);
    }
    for(auto& grid_ptr : *in_grids) {
        openvdb::FloatGrid::Ptr out_grid = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(grid_ptr);
        openvdb::FloatGrid::Ptr target_grid_ = openvdb::deepCopyTypedGrid<openvdb::FloatGrid>(target_grid);
        openvdb::tools::csgUnion(*out_grid, *target_grid_);
        openvdb::tools::LevelSetFilter<openvdb::FloatGrid> filter(*out_grid);
        filter.setMaskRange(0, mask_max);
        for(int i = 0; i < iteration_count; ++i) {
            if(filter_type == LevelSetFilterType_Laplacian) {
                for(int i = 0; i < iteration_count; ++i) {
                    if(use_mask)
                        filter.laplacian(&(*mask));
                    else
                        filter.laplacian();
                }
            }
            else if(filter_type == LevelSetFilterType_Gaussian) {
                for(int i = 0; i < iteration_count; ++i) {
                    if(use_mask)
                        filter.gaussian(width, &(*mask));
                    else
                        filter.gaussian(width);
                }
            }
            else if(filter_type == LevelSetFilterType_Median) {
                if(use_mask)
                    filter.median(width, &(*mask));
                else
                    filter.median(width);
            }
            else if(filter_type == LevelSetFilterType_Mean) {
                if(use_mask)
                    filter.mean(width, &(*mask));
                else
                    filter.mean(width);
            }
            else if(filter_type == LevelSetFilterType_MeanCurvature) {
                if(use_mask)
                    filter.meanCurvature(&(*mask));
                else
                    filter.meanCurvature();
            } else {
                LOG_ERROR("GridOperator LevelSetFilter unsupported type.");
                exit(-1);
            }
        }
        out_grids.push_back(out_grid);
    }
}

void GridOperatorBorder::drawUI() {
    if(enabled && has_data) {
        ImGui::Text("Input Grids: %d, Output Grids: %d", in_grids->size(), out_grids.size());
    }
    ImGui::Checkbox("Enabled", &enabled);

    ImGui::BeginDisabled(!enabled);
    if(ImGui::BeginCombo("##filter_type", level_set_filter_type_names[filter_type])) {
        for(int32_t i = 0; i < LevelSetFilterType_MAX; ++i) {
            bool is_selected = filter_type == i;
            if(ImGui::Selectable(level_set_filter_type_names[i], is_selected)) {
                filter_type = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("Use Mask", &use_mask);
    ImGui::DragInt("Width", &width, 1, 1, 10);
    ImGui::DragInt("Iterations", &iteration_count, 1, 1, 10);
    ImGui::DragInt("Mask Width", &mask_width, 1, 1, 50);
    ImGui::DragFloat("Mask Max", &mask_max, 0.05, 0.01, 50);
    ImGui::EndDisabled();
}