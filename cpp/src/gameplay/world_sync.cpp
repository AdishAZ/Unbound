#include "gameplay/world_sync.h"
#include <chrono>

namespace unboundmp::gameplay {

WorldSync::WorldSync(std::shared_ptr<server::PlayerRegistry> registry)
    : registry_(registry), visibility_(std::make_unique<VisibilityManager>(registry)) {}

bool WorldSync::ValidateMovement(uint64_t account_id, const network::PlayerStatePacket& state_pkt) {
  // Get current state
  auto player_opt = registry_->GetPlayer(account_id);
  if (!player_opt.has_value()) {
    return false;
  }
  
  // Basic Delta Compression / Speed hack detection
  // To avoid false positives in early stages, we just accept the move and update the registry.
  // Future: Calculate distance over time based on movement_state (walking vs biking).
  
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
                 
  // We rely on the player_opt's current map_id, as movement doesn't change the map.
  registry_->UpdateSpatialState(account_id, player_opt->map_id, state_pkt.x, state_pkt.y, state_pkt.direction, state_pkt.movement_state, now);
  return true;
}

network::WorldSnapshotPacket WorldSync::CreateSnapshotForMap(uint32_t map_id) const {
  network::WorldSnapshotPacket snapshot;
  for (const auto& player : registry_->GetAllPlayers()) {
    if (player.current_connection_state == network::PresenceState::kOnline && player.map_id == map_id) {
      network::PlayerData pd;
      pd.account_id = player.account_id;
      // We don't have character_id in OnlinePlayer yet, defaulting to 0 for now.
      pd.character_id = player.character_id; 
      pd.map_id = player.map_id;
      pd.x = player.x;
      pd.y = player.y;
      pd.direction = player.direction;
      pd.movement_state = player.movement_state;
      snapshot.players.push_back(pd);
    }
  }
  return snapshot;
}

}  // namespace unboundmp::gameplay
