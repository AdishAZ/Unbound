#pragma once

#include <memory>
#include <mutex>
#include "server/network/player_registry.h"
#include "gameplay/visibility_manager.h"
#include "network/packet.h"

namespace unboundmp::gameplay {

class WorldSync {
 public:
  explicit WorldSync(std::shared_ptr<server::PlayerRegistry> registry);

  // Validate a player's movement update against server rules
  // For now, accepts all moves (Delta compression/server-validation would go here)
  bool ValidateMovement(uint64_t account_id, const network::PlayerStatePacket& state_pkt);
  
  // Gets all players on the given map. Used for WorldSnapshots on map transition
  network::WorldSnapshotPacket CreateSnapshotForMap(uint32_t map_id) const;

 private:
  std::shared_ptr<server::PlayerRegistry> registry_;
  std::unique_ptr<VisibilityManager> visibility_;
};

}  // namespace unboundmp::gameplay
