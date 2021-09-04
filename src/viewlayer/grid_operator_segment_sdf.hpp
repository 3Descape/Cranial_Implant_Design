#pragma once
#include "grid_operator.hpp"

struct GridOperatorSegmentSDF : public GridOperator {
    GridOperatorSegmentSDF();
    virtual void execute() override;
    virtual void drawUI() override;
};