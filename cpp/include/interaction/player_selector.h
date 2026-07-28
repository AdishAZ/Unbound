#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "interaction/interaction_manager.h"

namespace unboundmp::interaction {

// PlayerSelector turns InteractionManager's candidate list into one chosen
// interaction target per local player - the player_id a future milestone
// would put in LinkSessionRequest.target_player_id (see packets.proto) once
// the local player confirms a trade/battle. It does not itself send or
// receive any packets.
class PlayerSelector {
 public:
  // Resolves the interaction target for `local_player_id` without
  // recording it as a persistent selection: the player directly faced
  // (distance 1, is_faced) if there is one, otherwise the single nearest
  // candidate from GetCandidates(). Returns nullopt if there are no
  // candidates at all. A faced player always wins over a merely-nearer
  // one - "facing" is the deliberate, no-ambiguity way to say "this one",
  // matching how a real GBA link interaction is initiated (you walk up to
  // and face the other player/NPC).
  std::optional<uint32_t> ResolveTarget(uint32_t local_player_id, const InteractionManager& manager) const;

  // Manually advances to the next candidate (by GetCandidates() order:
  // nearest first, ties by player_id) for `local_player_id`, wrapping
  // around after the last one, and records it as that player's current
  // selection. Useful for a "next target" input when more than one
  // candidate is in range and the local player isn't facing the one they
  // want. If the previous selection is no longer a candidate (they moved
  // out of range, changed maps, or disconnected), starts over from the
  // first candidate instead of failing. Returns nullopt if there are no
  // candidates at all.
  std::optional<uint32_t> CycleTarget(uint32_t local_player_id, const InteractionManager& manager);

  // The player_id `local_player_id` most recently selected via
  // CycleTarget(), if any and if still a valid candidate right now.
  // Selecting nothing (no prior CycleTarget call, or ClearSelection())
  // returns nullopt rather than falling back to ResolveTarget()'s
  // automatic behavior - the two are deliberately separate concepts.
  std::optional<uint32_t> GetSelection(uint32_t local_player_id, const InteractionManager& manager) const;

  void ClearSelection(uint32_t local_player_id);

 private:
  std::unordered_map<uint32_t, uint32_t> selected_target_by_local_player_;
};

}  // namespace unboundmp::interaction
