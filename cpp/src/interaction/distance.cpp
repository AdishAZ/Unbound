#include "interaction/distance.h"

#include <cstdlib>

namespace unboundmp::interaction {

bool SameMap(const PlayerSnapshot& a, const PlayerSnapshot& b) {
  return a.map_bank == b.map_bank && a.map_number == b.map_number;
}

int32_t TileDistance(const PlayerSnapshot& a, const PlayerSnapshot& b) {
  const int32_t dx = a.x - b.x;
  const int32_t dy = a.y - b.y;
  return std::abs(dx) + std::abs(dy);
}

bool WithinRange(const PlayerSnapshot& a, const PlayerSnapshot& b, int32_t max_distance) {
  return SameMap(a, b) && TileDistance(a, b) <= max_distance;
}

}  // namespace unboundmp::interaction
