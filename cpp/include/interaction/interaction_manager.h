#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "interaction/interaction_types.h"
#include "packets.pb.h"

namespace unboundmp::interaction {

struct InteractionManagerConfig {
  // How far away (Manhattan tiles, same map only) another player can be
  // and still show up as a candidate at all. This is a "who's nearby"
  // radius for selection/UI purposes, not a claim that every candidate is
  // interactable right now - actually starting a trade/battle additionally
  // requires is_faced (see PlayerSelector), which this radius has nothing
  // to do with.
  int32_t max_candidate_distance = 5;
};

// Tracks every player's latest known position/facing/map from
// protocol::PlayerStateUpdate packets (local player included - it's fed
// the local echo the same way FollowerManager is) and answers "who is
// near/faced by player X" queries against that tracked state.
//
// This class does no memory reading, no networking, and no rendering of
// its own - it's the game-logic layer that turns a stream of
// PlayerStateUpdate packets (already flowing through the system since
// Milestone 9) into "who can player X interact with", for a future
// milestone to turn into an actual LinkSessionRequest (trade/battle) once
// a target is chosen.
class InteractionManager {
 public:
  explicit InteractionManager(InteractionManagerConfig config = {}) : config_(config) {}

  // Records/updates one player's tracked snapshot from an incoming packet.
  void OnPlayerStateUpdate(const protocol::PlayerStateUpdate& update);

  // Stops tracking a player (e.g. on disconnect). Safe to call for a
  // player_id that was never tracked.
  void RemovePlayer(uint32_t player_id);

  std::optional<PlayerSnapshot> GetSnapshot(uint32_t player_id) const;

  // Every other tracked player within max_candidate_distance tiles of
  // `local_player_id`, on the same map. Sorted nearest-first; ties are
  // broken by ascending player_id so results are deterministic (useful for
  // both tests and predictable cycling in PlayerSelector). Returns an
  // empty vector if `local_player_id` isn't tracked, has no map yet, or no
  // one else qualifies.
  std::vector<InteractionCandidate> GetCandidates(uint32_t local_player_id) const;

 private:
  std::unordered_map<uint32_t, PlayerSnapshot> players_;
  InteractionManagerConfig config_;
};

}  // namespace unboundmp::interaction
