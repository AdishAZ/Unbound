#pragma once

#include "memory/direction_reader.h"
#include "packets.pb.h"

namespace unboundmp::gameplay {

// Tile-space step for one move in a given direction. (0, 0) for
// DIRECTION_UNSPECIFIED, since "no direction" has no meaningful step.
// Uses the same +y-is-down tile convention as memory::Position /
// protocol::PlayerStateUpdate (see position_reader.h) - not an assumption
// introduced here.
struct StepVector {
  int32_t dx = 0;
  int32_t dy = 0;
};

StepVector StepFor(protocol::Direction direction);

// The direction opposite `direction` (e.g. DOWN <-> UP). Used to find
// "the tile the trainer just came from", which is where the follower
// should be heading - see movement_trail.h. DIRECTION_UNSPECIFIED maps to
// itself.
protocol::Direction Opposite(protocol::Direction direction);

// Translates the local-memory-reader's FacingDirection (see
// memory/direction_reader.h) into the wire protocol's Direction enum, so
// gameplay code and outgoing PlayerStateUpdate packets only ever deal with
// one direction type. FacingDirection::kUnknown maps to
// DIRECTION_UNSPECIFIED, matching how an unconfigured/unrecognized raw
// value should be treated (rather than guessing a default facing).
protocol::Direction ToProtocolDirection(memory::FacingDirection direction);

}  // namespace unboundmp::gameplay
