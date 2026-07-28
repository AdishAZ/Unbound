#include "render/camera_controller.h"
#include "render/tile_provider.h"
#include <cmath>
#include <algorithm>

namespace unboundmp::render {

CameraController::CameraController(Camera& camera) : camera_(camera) {}

void CameraController::Update(float dt) {
    float world_x = camera_.GetX();
    float world_y = camera_.GetY();
    
    if (world_x != target_x_ || world_y != target_y_) {
        // Simple linear interpolation / easing
        world_x += (target_x_ - world_x) * follow_speed_ * dt;
        world_y += (target_y_ - world_y) * follow_speed_ * dt;
        
        // Snap if close enough
        if (std::abs(world_x - target_x_) < 0.01f) world_x = target_x_;
        if (std::abs(world_y - target_y_) < 0.01f) world_y = target_y_;
    }
    
    if (tile_provider_) {
        const auto& win = camera_.Window();
        float view_w = static_cast<float>(win.columns);
        float view_h = static_cast<float>(win.rows);
        
        float border_w = static_cast<float>(tile_provider_->GetBorderWidth());
        float border_h = static_cast<float>(tile_provider_->GetBorderHeight());
        float map_w = static_cast<float>(tile_provider_->GetMapWidth());
        float map_h = static_cast<float>(tile_provider_->GetMapHeight());
        
        float min_x = -border_w + (view_w / 2.0f);
        float min_y = -border_h + (view_h / 2.0f);
        float max_x = map_w + border_w - (view_w / 2.0f);
        float max_y = map_h + border_h - (view_h / 2.0f);
        
        if (max_x < min_x) { float mid = (min_x + max_x) / 2.0f; min_x = mid; max_x = mid; }
        if (max_y < min_y) { float mid = (min_y + max_y) / 2.0f; min_y = mid; max_y = mid; }
        
        SetBounds(min_x, min_y, max_x, max_y);
    }

    if (has_bounds_) {
        world_x = std::clamp(world_x, min_x_, max_x_);
        world_y = std::clamp(world_y, min_y_, max_y_);
    }
    
    camera_.SetPosition(world_x, world_y);
}

} // namespace unboundmp::render
