#include "interaction/interaction_manager.h"

#include <algorithm>

#include "interaction/distance.h"
#include "interaction/facing.h"

namespace unboundmp::interaction {

void InteractionManager::OnPlayerStateUpdate(const protocol::PlayerStateUpdate& update) {
  PlayerSnapshot& snapshot = players_[update.player_id()];
  snapshot.player_id = update.player_id();
  snapshot.map_bank = update.map_bank();
  snapshot.map_number = update.map_number();
  snapshot.x = update.x();
  snapshot.y = update.y();
  snapshot.facing = update.facing();
}

void InteractionManager::RemovePlayer(uint32_t player_id) { players_.erase(player_id); }

std::optional<PlayerSnapshot> InteractionManager::GetSnapshot(uint32_t player_id) const {
  auto it = players_.find(player_id);
  if (it == players_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::vector<InteractionCandidate> InteractionManager::GetCandidates(uint32_t local_player_id) const {
  std::vector<InteractionCandidate> candidates;

  auto local_it = players_.find(local_player_id);
  if (local_it == players_.end()) {
    return candidates;
  }
  const PlayerSnapshot& local = local_it->second;

  for (const auto& [player_id, snapshot] : players_) {
    if (player_id == local_player_id) {
      continue;
    }
    if (!WithinRange(local, snapshot, config_.max_candidate_distance)) {
      continue;
    }

    InteractionCandidate candidate;
    candidate.player_id = player_id;
    candidate.distance = TileDistance(local, snapshot);
    candidate.is_faced = IsFacing(local, snapshot);
    candidates.push_back(candidate);
  }

  std::sort(candidates.begin(), candidates.end(), [](const InteractionCandidate& a, const InteractionCandidate& b) {
    if (a.distance != b.distance) {
      return a.distance < b.distance;
    }
    return a.player_id < b.player_id;
  });

  return candidates;
}

}  // namespace unboundmp::interaction
