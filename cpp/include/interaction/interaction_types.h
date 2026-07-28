#pragma once

#include <cstdint>

#include "packets.pb.h"

namespace unboundmp::interaction {

// The subset of a player's world state this layer needs to decide who is
// near/facing whom. Deliberately smaller than protocol::PlayerStateUpdate -
// this is a value type this layer owns, not the wire packet itself, so
// InteractionManager isn't coupled to protobuf field access everywhere it's
// used (e.g. by future rendering/UI code that just wants a plain struct).
struct PlayerSnapshot {
  uint32_t player_id = 0;
  uint32_t map_bank = 0;
  uint32_t map_number = 0;
  int32_t x = 0;
  int32_t y = 0;
  protocol::Direction facing = protocol::DIRECTION_UNSPECIFIED;
};

// One other player considered as a possible interaction target for some
// local player - who they are, how far away (Manhattan tiles, only
// meaningful when both players are on the same map), and whether the local
// player is standing exactly one tile away and facing directly at them.
struct InteractionCandidate {
  uint32_t player_id = 0;
  int32_t distance = 0;
  bool is_faced = false;
};

}  // namespace unboundmp::interaction
