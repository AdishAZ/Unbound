#include "gameplay/visibility_manager.h"
#include <cmath>

namespace unboundmp::gameplay {

std::vector<server::OnlinePlayer> VisibilityManager::GetObserversFor(uint64_t source_account_id, float visibility_radius) const {
  std::vector<server::OnlinePlayer> observers;
  
  auto source_opt = registry_->GetPlayer(source_account_id);
  if (!source_opt.has_value()) {
    return observers;
  }
  
  const auto& source = source_opt.value();
  float radius_sq = visibility_radius * visibility_radius;
  
  for (const auto& player : registry_->GetAllPlayers()) {
    // Don't observe yourself
    if (player.account_id == source_account_id) {
      continue;
    }
    
    // Must be online and on the same map
    if (player.current_connection_state != network::PresenceState::kOnline || player.map_id != source.map_id) {
      continue;
    }
    
    // Distance check
    float dx = player.x - source.x;
    float dy = player.y - source.y;
    float dist_sq = (dx * dx) + (dy * dy);
    
    if (dist_sq <= radius_sq) {
      observers.push_back(player);
    }
  }
  
  return observers;
}

}  // namespace unboundmp::gameplay
