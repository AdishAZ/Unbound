#include "gameplay/movement_sync.h"
#include <algorithm>

namespace unboundmp::gameplay {

MovementSync::Vector2D MovementSync::Lerp(const Vector2D& start, const Vector2D& end, float t) {
  // Clamp t to [0, 1]
  t = std::max(0.0f, std::min(1.0f, t));
  return {
    start.x + (end.x - start.x) * t,
    start.y + (end.y - start.y) * t
  };
}

float MovementSync::CalculateInterpolationFactor(int64_t elapsed_ms, int expected_hz) {
  if (expected_hz <= 0) return 1.0f;
  float frame_time_ms = 1000.0f / expected_hz;
  return static_cast<float>(elapsed_ms) / frame_time_ms;
}

}  // namespace unboundmp::gameplay
