#include "grid_operator_segment_sdf.hpp"

#include <algorithm>
#include <imgui/imgui.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetUtil.h>

GridOperatorSegmentSDF::GridOperatorSegmentSDF()
    : GridOperator(GridOperator_SegmentSDF) {}

void GridOperatorSegmentSDF::execute() {
    out_grids.clear();
    for(auto& grid : *in_grids) {
        std::vector<openvdb::FloatGrid::Ptr> grids;
        openvdb::tools::segmentSDF(*grid, grids);
        out_grids.insert(out_grids.end(), grids.begin(), grids.end());
    }
    struct sort_operator{
        inline bool operator()(const openvdb::FloatGrid::Ptr& a, const openvdb::FloatGrid::Ptr& b) {
            return a->activeVoxelCount() > b->activeVoxelCount();
        }
    };
    std::sort(out_grids.begin(), out_grids.end(), sort_operator());
}

void GridOperatorSegmentSDF::drawUI() {
    if(enabled && has_data) {
        ImGui::Text("Input Grids: %d, Output Grids: %d", in_grids->size(), out_grids.size());
    }
    ImGui::Checkbox("Enabled", &enabled);
}