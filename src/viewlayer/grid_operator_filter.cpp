#include "grid_operator_filter.hpp"
#include <imgui/imgui.h>
#include <openvdb/openvdb.h>

GridOperatorFilter::GridOperatorFilter()
    : GridOperator(GridOperator_Filter) {}

void GridOperatorFilter::execute() {
    out_grids.clear();
    if(filter_type == FilterType_TakeCount) {
        if(take_count_offset < in_grids->size()) {
            for(int i = take_count_offset, count = 0; i < in_grids->size() && count < take_count; ++i, ++count) {
                out_grids.push_back((*in_grids)[i]);
            }
        }
    } else if(filter_type == FilterType_ActiveVoxelCount_GreaterThan) {
        for(auto& grid : *in_grids) {
            if(grid->activeVoxelCount() >= threshold_active_voxel_count)
                out_grids.push_back(grid);
        }
    } else if(filter_type == FilterType_ActiveVoxelCount_LessThan) {
        for(auto& grid : *in_grids) {
            if(grid->activeVoxelCount() <= threshold_active_voxel_count)
                out_grids.push_back(grid);
        }
    }
}

void GridOperatorFilter::drawUI() {
    ImGui::Checkbox("Enabled", &enabled);
    if(enabled && has_data) {
        ImGui::Text("Input Grids: %d, Output Grids: %d", in_grids->size(), out_grids.size());
    }
    ImGui::BeginDisabled(!enabled);
        if(ImGui::BeginCombo("##filter_type", filter_type_names[filter_type])) {
            for(int32_t i = 0; i < FilterType_MAX; ++i) {
                bool is_selected = filter_type == i;
                if(ImGui::Selectable(filter_type_names[i], is_selected)) {
                    filter_type = i;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if(filter_type == FilterType_ActiveVoxelCount_GreaterThan || filter_type == FilterType_ActiveVoxelCount_LessThan) {
            ImGui::DragInt("Active Voxel Count", (int*)&threshold_active_voxel_count, 1000, 0, 100000);
        } else if(filter_type == FilterType_TakeCount) {
            ImGui::DragInt("Count", (int*)&take_count, 1, 0, in_grids ? in_grids->size() : 10);
            ImGui::DragInt("Offset", (int*)&take_count_offset, 1, 0, in_grids ? in_grids->size() - 1 : 9);
        }
    ImGui::EndDisabled();
}