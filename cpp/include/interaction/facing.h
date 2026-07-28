#pragma once

#include "interaction/interaction_types.h"

namespace unboundmp::interaction {

// A tile coordinate. Kept as its own tiny type (rather than reusing
// memory::Position or gameplay::StepVector) so this header doesn't have to
// pull in the memory layer just to describe "one tile".
struct TileCoord {
  int32_t x = 0;
  int32_t y = 0;
};

// The single tile directly in front of `observer` - one step from its own
// position in the direction it's facing. Returns `observer`'s own tile
// unchanged when facing is DIRECTION_UNSPECIFIED, matching
// gameplay::StepFor's "no direction has no meaningful step" convention -
// nobody is considered "faced" by a player with no known facing.
TileCoord GetFacedTile(const PlayerSnapshot& observer);

// True iff `observer` and `other` are on the same map and `other` is
// standing exactly on the tile `observer` is facing (i.e. one step away,
// in the facing direction). Facing is one-directional: this says nothing
// about whether `other` is facing back at `observer`.
bool IsFacing(const PlayerSnapshot& observer, const PlayerSnapshot& other);

}  // namespace unboundmp::interaction
