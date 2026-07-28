#include "interaction/player_selector.h"

#include <algorithm>

namespace unboundmp::interaction {

std::optional<uint32_t> PlayerSelector::ResolveTarget(uint32_t local_player_id,
                                                        const InteractionManager& manager) const {
  const std::vector<InteractionCandidate> candidates = manager.GetCandidates(local_player_id);
  if (candidates.empty()) {
    return std::nullopt;
  }

  for (const InteractionCandidate& candidate : candidates) {
    if (candidate.is_faced) {
      return candidate.player_id;
    }
  }
  // GetCandidates() is sorted nearest-first, so the first entry is the
  // nearest candidate when nobody is directly faced.
  return candidates.front().player_id;
}

std::optional<uint32_t> PlayerSelector::CycleTarget(uint32_t local_player_id, const InteractionManager& manager) {
  const std::vector<InteractionCandidate> candidates = manager.GetCandidates(local_player_id);
  if (candidates.empty()) {
    selected_target_by_local_player_.erase(local_player_id);
    return std::nullopt;
  }

  auto it = selected_target_by_local_player_.find(local_player_id);
  if (it == selected_target_by_local_player_.end()) {
    const uint32_t first = candidates.front().player_id;
    selected_target_by_local_player_[local_player_id] = first;
    return first;
  }

  const auto current = std::find_if(candidates.begin(), candidates.end(),
                                     [&](const InteractionCandidate& c) { return c.player_id == it->second; });
  uint32_t next;
  if (current == candidates.end()) {
    // Previous selection dropped out of range/disconnected/changed maps -
    // start over rather than getting stuck on a stale target.
    next = candidates.front().player_id;
  } else {
    const auto next_it = std::next(current);
    next = (next_it == candidates.end()) ? candidates.front().player_id : next_it->player_id;
  }

  it->second = next;
  return next;
}

std::optional<uint32_t> PlayerSelector::GetSelection(uint32_t local_player_id,
                                                        const InteractionManager& manager) const {
  auto it = selected_target_by_local_player_.find(local_player_id);
  if (it == selected_target_by_local_player_.end()) {
    return std::nullopt;
  }

  const std::vector<InteractionCandidate> candidates = manager.GetCandidates(local_player_id);
  const bool still_valid =
      std::any_of(candidates.begin(), candidates.end(), [&](const InteractionCandidate& c) { return c.player_id == it->second; });
  if (!still_valid) {
    return std::nullopt;
  }
  return it->second;
}

void PlayerSelector::ClearSelection(uint32_t local_player_id) { selected_target_by_local_player_.erase(local_player_id); }

}  // namespace unboundmp::interaction
