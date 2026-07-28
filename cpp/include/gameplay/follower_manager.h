#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "gameplay/follower_animation.h"
#include "gameplay/follower_state.h"
#include "gameplay/movement_trail.h"
#include "memory/follower_reader.h"
#include "packets.pb.h"

namespace unboundmp::gameplay {

struct FollowerManagerConfig {
  // How many tiles behind the trainer the follower trails. See
  // movement_trail.h for why 1 is the default.
  size_t lag_steps = 1;
  size_t max_trail_history = 8;
  MovementCadence cadence{};
};

// Owns one FollowerVisualState per player (local or remote) and keeps it
// up to date from three kinds of input:
//   - the local player's own memory::FollowerState (species/visible/shiny,
//     read from RAM by memory::FollowerReader in a future integration
//     milestone - this class only consumes the already-decoded struct)
//   - protocol::FollowerUpdate packets (species sync - both the local
//     echo and every remote player's follower)
//   - protocol::PlayerStateUpdate packets (position/direction/movement
//     mode - drives the MovementTrail offset and walk animation for every
//     player, local and remote alike, since the local player's own
//     follower trails the local player's own trainer exactly the same way
//     a remote follower trails a remote trainer)
//
// This class does no memory reading, no networking, and no rendering
// itself - it is pure game-state logic sitting between those layers,
// consuming their already-defined data types (memory::FollowerState,
// protocol::FollowerUpdate, protocol::PlayerStateUpdate) and producing
// FollowerVisualState for a future rendering milestone to consume.
class FollowerManager {
 public:
  explicit FollowerManager(FollowerManagerConfig config = {}) : config_(config) {}

  // Feeds the local player's freshly-read follower state in. Returns a
  // populated FollowerUpdate iff species/visible/shiny changed since the
  // last call (nullopt otherwise) - the caller sends this over the
  // network only when something is actually different, so an unchanging
  // follower doesn't produce a packet every tick. (This is the
  // "species synchronization" half of the milestone: deciding *when* to
  // sync, not the wire format itself, which already exists in
  // packets.proto.)
  std::optional<protocol::FollowerUpdate> UpdateLocalFollower(uint32_t local_player_id,
                                                                const memory::FollowerState& state);

  // Applies an incoming FollowerUpdate - the local echo of the call above,
  // or a remote player's - to that player's tracked visual state.
  void OnFollowerUpdate(const protocol::FollowerUpdate& update);

  // Applies an incoming PlayerStateUpdate: records the trainer's new tile
  // into that player's MovementTrail (resetting it across a map
  // transition instead of letting the follower walk a phantom path across
  // maps), and updates the movement mode/is_moving bookkeeping the walk
  // animation needs.
  void OnPlayerStateUpdate(const protocol::PlayerStateUpdate& update);

  // Advances every tracked player's walk animation and, whenever a step
  // completes, pulls the next lagged target tile off that player's
  // MovementTrail. Call once per frame/tick from the main loop with the
  // elapsed time since the last call.
  void Tick(uint32_t elapsed_ms);

  std::optional<FollowerVisualState> GetVisualState(uint32_t player_id) const;
  std::vector<FollowerVisualState> AllVisualStates() const;

  void RemovePlayer(uint32_t player_id);

 private:
  struct PlayerFollower {
    FollowerVisualState visual;
    MovementTrail trail;
    FollowerAnimator animator;
    uint32_t map_bank = 0;
    uint32_t map_number = 0;
    bool has_map = false;
    protocol::MovementMode last_mode = protocol::MOVEMENT_MODE_UNSPECIFIED;
    bool last_is_moving = false;

    explicit PlayerFollower(const FollowerManagerConfig& config)
        : trail(config.lag_steps, config.max_trail_history), animator(config.cadence) {}
  };

  PlayerFollower& GetOrCreate(uint32_t player_id);

  std::unordered_map<uint32_t, PlayerFollower> players_;
  FollowerManagerConfig config_;
};

}  // namespace unboundmp::gameplay
