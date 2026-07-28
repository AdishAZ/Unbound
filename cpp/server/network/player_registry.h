#pragma once

#include "network/connection.h"
#include "network/packet.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <memory>
#include "persistence/dirty_flag_manager.h"

namespace unboundmp::server {

struct OnlinePlayer {
  std::string session_token;
  uint32_t connection_id = 0;
  uint64_t account_id = 0;
  uint64_t character_id = 0;
  int64_t login_time = 0;
  int64_t last_heartbeat = 0;
  network::PresenceState current_connection_state = network::PresenceState::kOffline;
  
  uint32_t map_id = 0;
  float x = 0.0f;
  float y = 0.0f;
  uint8_t direction = 0;
  uint8_t movement_state = 0;
  int64_t last_sync_time = 0;
  
  // Stage 8 Persistence
  std::shared_ptr<persistence::DirtyFlagManager> dirty_manager;
  std::shared_ptr<network::PlayerDataSyncPacket> persistence_state;
};

class PlayerRegistry {
 public:
  PlayerRegistry() = default;

  void AddPlayer(const OnlinePlayer& player);
  void RemovePlayer(uint64_t account_id);
  void UpdatePresence(uint64_t account_id, network::PresenceState state);
  void UpdateHeartbeat(uint64_t account_id, int64_t timestamp);
  void UpdateSpatialState(uint64_t account_id, uint32_t map_id, float x, float y, uint8_t direction, uint8_t movement_state, int64_t timestamp);

  std::optional<OnlinePlayer> GetPlayer(uint64_t account_id);
  std::optional<OnlinePlayer> GetPlayerByToken(const std::string& token);
  std::optional<OnlinePlayer> GetPlayerByConnection(uint32_t connection_id);

  std::vector<OnlinePlayer> GetAllPlayers();

 private:
  std::mutex registry_mutex_;
  std::unordered_map<uint64_t, OnlinePlayer> players_by_account_;
  std::unordered_map<std::string, uint64_t> account_by_token_;
  std::unordered_map<uint32_t, uint64_t> account_by_connection_;
};

}  // namespace unboundmp::server
