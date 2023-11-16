#include "grid_operator.hpp"

#include <vector>
#include <openvdb/openvdb.h>

GridOperator::GridOperator(int type_) :
    type(type_) {};

std::vector<openvdb::FloatGrid::Ptr> grid_operator_execute_all(std::vector<GridOperator*>& grid_operations, const std::vector<openvdb::FloatGrid::Ptr>& in_grids) {
    std::vector<openvdb::FloatGrid::Ptr>const* grids_ptr = &in_grids;
    if(grid_operations.size() > 1) {
        for(GridOperator* grid_operator : grid_operations) {
            if(!grid_operator->enabled) continue;
            grid_operator->in_grids = grids_ptr;
            grids_ptr = &grid_operator->out_grids;
        }
    }

    for(GridOperator* grid_operator : grid_operations) {
        grid_operator->execute();
        grid_operator->has_data = true;
    }

    return *grids_ptr;
}