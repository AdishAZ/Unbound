#include "network/player_registry.h"
#include <algorithm>

namespace unboundmp::server {

void PlayerRegistry::AddPlayer(const OnlinePlayer& player) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  
  // Remove existing mappings if they exist
  auto it = players_by_account_.find(player.account_id);
  if (it != players_by_account_.end()) {
    account_by_token_.erase(it->second.session_token);
    account_by_connection_.erase(it->second.connection_id);
  }

  players_by_account_[player.account_id] = player;
  account_by_token_[player.session_token] = player.account_id;
  account_by_connection_[player.connection_id] = player.account_id;
}

void PlayerRegistry::RemovePlayer(uint64_t account_id) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  
  auto it = players_by_account_.find(account_id);
  if (it != players_by_account_.end()) {
    account_by_token_.erase(it->second.session_token);
    account_by_connection_.erase(it->second.connection_id);
    players_by_account_.erase(it);
  }
}

void PlayerRegistry::UpdatePresence(uint64_t account_id, network::PresenceState state) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = players_by_account_.find(account_id);
  if (it != players_by_account_.end()) {
    it->second.current_connection_state = state;
  }
}

void PlayerRegistry::UpdateHeartbeat(uint64_t account_id, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = players_by_account_.find(account_id);
  if (it != players_by_account_.end()) {
    it->second.last_heartbeat = timestamp;
  }
}

void PlayerRegistry::UpdateSpatialState(uint64_t account_id, uint32_t map_id, float x, float y, uint8_t direction, uint8_t movement_state, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = players_by_account_.find(account_id);
  if (it != players_by_account_.end()) {
    // Basic timestamp validation (ignore old packets)
    if (timestamp >= it->second.last_sync_time) {
      it->second.map_id = map_id;
      it->second.x = x;
      it->second.y = y;
      it->second.direction = direction;
      it->second.movement_state = movement_state;
      it->second.last_sync_time = timestamp;
    }
  }
}

std::optional<OnlinePlayer> PlayerRegistry::GetPlayer(uint64_t account_id) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = players_by_account_.find(account_id);
  if (it != players_by_account_.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<OnlinePlayer> PlayerRegistry::GetPlayerByToken(const std::string& token) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = account_by_token_.find(token);
  if (it != account_by_token_.end()) {
    return players_by_account_[it->second];
  }
  return std::nullopt;
}

std::optional<OnlinePlayer> PlayerRegistry::GetPlayerByConnection(uint32_t connection_id) {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  auto it = account_by_connection_.find(connection_id);
  if (it != account_by_connection_.end()) {
    return players_by_account_[it->second];
  }
  return std::nullopt;
}

std::vector<OnlinePlayer> PlayerRegistry::GetAllPlayers() {
  std::lock_guard<std::mutex> lock(registry_mutex_);
  std::vector<OnlinePlayer> players;
  players.reserve(players_by_account_.size());
  for (const auto& [acc_id, player] : players_by_account_) {
    players.push_back(player);
  }
  return players;
}

}  // namespace unboundmp::server
