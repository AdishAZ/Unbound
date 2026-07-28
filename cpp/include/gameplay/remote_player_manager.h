#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "network/packet.h"
#include "gameplay/position_interpolator.h"

namespace unboundmp::network { class NetworkClock; }

namespace unboundmp::gameplay {

struct RemotePlayer {
  uint64_t account_id = 0;
  uint64_t character_id = 0;
  uint32_t map_id = 0;
  
  // Interpolation state
  PositionInterpolator interpolator;
  float current_x = 0.0f;
  float current_y = 0.0f;
  
  uint8_t direction = 0;
  uint8_t movement_state = 0;
  
  int64_t last_update_time = 0;
};

// Manages the state of remote players on the client side
class RemotePlayerManager {
 public:
  void Initialize(network::NetworkClock* clock);
  void HandleSpawn(const network::PlayerSpawnPacket& pkt, int64_t timestamp);
  void HandleDespawn(const network::PlayerDespawnPacket& pkt);
  void HandleStateUpdate(const network::PlayerStatePacket& pkt, int64_t timestamp);
  void HandlePlayerMove(const network::PlayerMovePacket& pkt, int64_t timestamp);
  void HandlePlayerDirection(const network::PlayerDirectionPacket& pkt, int64_t timestamp);
  void HandleMapTransition(const network::MapTransitionPacket& pkt, int64_t timestamp);
  void HandleWorldSnapshot(const network::WorldSnapshotPacket& pkt, int64_t timestamp);

  // Called every frame by the client to step interpolation
  void Update(float dt);
  
  std::vector<RemotePlayer> GetPlayersOnMap(uint32_t map_id) const;
  std::vector<RemotePlayer> GetAllPlayers() const;
  size_t GetPlayerCount() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return players_.size();
  }

 private:
  mutable std::mutex mutex_;
  std::unordered_map<uint64_t, RemotePlayer> players_;
};

}  // namespace unboundmp::gameplay
