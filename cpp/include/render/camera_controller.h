#pragma once

#include "render/camera.h"

namespace unboundmp::render {

class CameraController {
public:
    CameraController(Camera& camera);

    void SetTarget(float x, float y) {
        target_x_ = x;
        target_y_ = y;
    }
    
    void SetBounds(float min_x, float min_y, float max_x, float max_y) {
        has_bounds_ = true;
        min_x_ = min_x;
        min_y_ = min_y;
        max_x_ = max_x;
        max_y_ = max_y;
    }
    
    void ClearBounds() { has_bounds_ = false; }
    
    void Update(float dt);

    float GetTargetX() const { return target_x_; }
    float GetTargetY() const { return target_y_; }

    void SetTileProvider(class TileProvider* provider) { tile_provider_ = provider; }

private:
    Camera& camera_;
    class TileProvider* tile_provider_ = nullptr;
    
    float target_x_ = 0.0f;
    float target_y_ = 0.0f;
    float follow_speed_ = 5.0f;
    
    bool has_bounds_ = false;
    float min_x_ = 0.0f, min_y_ = 0.0f, max_x_ = 0.0f, max_y_ = 0.0f;
};

} // namespace unboundmp::render
