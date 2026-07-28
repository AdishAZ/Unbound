#pragma once

namespace unboundmp::gameplay {

// Encapsulates linear interpolation logic for entity movement.
class PositionInterpolator {
public:
    PositionInterpolator() = default;

    // Call this when receiving a new target position from the server.
    void SetTarget(float current_x, float current_y, float target_x, float target_y);

    // Call every frame with the delta time.
    // Returns the newly interpolated position (x, y).
    void Update(float dt, float& out_x, float& out_y);

    // Immediately snap to a position, canceling interpolation.
    void Snap(float x, float y);

    bool IsInterpolating() const { return time_accumulated_ < duration_; }

private:
    float start_x_ = 0.0f;
    float start_y_ = 0.0f;
    float target_x_ = 0.0f;
    float target_y_ = 0.0f;
    
    float time_accumulated_ = 0.0f;
    float duration_ = 0.05f; // 50ms interpolation window
};

} // namespace unboundmp::gameplay
