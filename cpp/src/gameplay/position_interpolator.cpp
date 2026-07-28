#include "gameplay/position_interpolator.h"
#include <algorithm>

namespace unboundmp::gameplay {

void PositionInterpolator::SetTarget(float current_x, float current_y, float target_x, float target_y) {
    start_x_ = current_x;
    start_y_ = current_y;
    target_x_ = target_x;
    target_y_ = target_y;
    time_accumulated_ = 0.0f;
}

void PositionInterpolator::Update(float dt, float& out_x, float& out_y) {
    if (time_accumulated_ >= duration_) {
        out_x = target_x_;
        out_y = target_y_;
        return;
    }

    time_accumulated_ += dt;
    float t = std::clamp(time_accumulated_ / duration_, 0.0f, 1.0f);
    
    out_x = start_x_ + (target_x_ - start_x_) * t;
    out_y = start_y_ + (target_y_ - start_y_) * t;
}

void PositionInterpolator::Snap(float x, float y) {
    start_x_ = x;
    start_y_ = y;
    target_x_ = x;
    target_y_ = y;
    time_accumulated_ = duration_;
}

} // namespace unboundmp::gameplay
