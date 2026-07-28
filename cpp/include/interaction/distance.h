#pragma once

#include "interaction/interaction_types.h"

namespace unboundmp::interaction {

// True iff both snapshots report the same (map_bank, map_number). Distance
// and facing are meaningless across maps - two players standing on the
// same (x, y) in different towns are not "on top of each other" - so every
// other function in this file/interaction_manager.h checks this first.
bool SameMap(const PlayerSnapshot& a, const PlayerSnapshot& b);

// Manhattan tile distance between `a` and `b`. Manhattan (not Euclidean or
// Chebyshev) matches the engine's 4-directional grid movement, where a
// diagonal isn't a single step. Does not itself check SameMap() - callers
// that care about cross-map results should check that separately (see
// WithinRange(), which does).
int32_t TileDistance(const PlayerSnapshot& a, const PlayerSnapshot& b);

// True iff `a` and `b` are on the same map and TileDistance(a, b) is no
// more than `max_distance` tiles.
bool WithinRange(const PlayerSnapshot& a, const PlayerSnapshot& b, int32_t max_distance);

}  // namespace unboundmp::interaction
