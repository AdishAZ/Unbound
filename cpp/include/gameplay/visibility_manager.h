#pragma once

#include <vector>
#include <memory>
#include "server/network/player_registry.h"

namespace unboundmp::gameplay {

class VisibilityManager {
 public:
  explicit VisibilityManager(std::shared_ptr<server::PlayerRegistry> registry)
      : registry_(registry) {}

  // Returns all players who should receive updates from the source_player.
  // Generally, this means players on the same map within visibility_radius.
  std::vector<server::OnlinePlayer> GetObserversFor(uint64_t source_account_id, float visibility_radius = 20.0f) const;

 private:
  std::shared_ptr<server::PlayerRegistry> registry_;
};

}  // namespace unboundmp::gameplay
