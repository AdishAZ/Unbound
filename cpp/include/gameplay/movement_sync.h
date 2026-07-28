#pragma once

#include <cstdint>

namespace unboundmp::gameplay {

// MovementSync handles math for interpolating positions over time
class MovementSync {
 public:
  struct Vector2D {
    float x;
    float y;
  };

  // Linear interpolation between start and end by factor t (0.0 to 1.0)
  static Vector2D Lerp(const Vector2D& start, const Vector2D& end, float t);

  // Calculates a smooth t value based on elapsed time and expected sync rate
  // Expected sync rate is in Hz (e.g. 20 Hz = 50ms per tick)
  static float CalculateInterpolationFactor(int64_t elapsed_ms, int expected_hz = 20);
};

}  // namespace unboundmp::gameplay
