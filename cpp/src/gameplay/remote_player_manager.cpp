#include "gameplay/remote_player_manager.h"
#include "core/log_manager.h"
#include "network/client_packet_dispatcher.h"
#include <iostream>
#include "network/network_clock.h"
#include "gameplay/movement_sync.h"
#include "ui/profiler.h"

namespace unboundmp::gameplay {

void RemotePlayerManager::Initialize(network::NetworkClock* clock) {
    auto& dispatcher = network::ClientPacketDispatcher::GetInstance();
    
    dispatcher.Subscribe(network::PacketType::kWorldSnapshot, [this, clock](const network::Packet& p) {
        auto pkt = network::WorldSnapshotPacket::Deserialize(p.payload);
        if (clock) clock->Sync(pkt.server_tick, pkt.server_time_ms);
        unboundmp::ui::Profiler::Instance().SetServerTick(pkt.server_tick);
        HandleWorldSnapshot(pkt, clock ? clock->GetServerTimeMs() : 0);
    });

    dispatcher.Subscribe(network::PacketType::kPlayerSpawn, [this, clock](const network::Packet& p) {
        auto pkt = network::PlayerSpawnPacket::Deserialize(p.payload);
        std::cerr << "[RPM] kPlayerSpawn account_id=" << pkt.account_id << " pos=(" << pkt.x << "," << pkt.y << ") map=" << pkt.map_id << std::endl;
        HandleSpawn(pkt, clock ? clock->GetServerTimeMs() : 0);
    });

    dispatcher.Subscribe(network::PacketType::kPlayerDespawn, [this](const network::Packet& p) {
        auto pkt = network::PlayerDespawnPacket::Deserialize(p.payload);
        HandleDespawn(pkt);
    });

    dispatcher.Subscribe(network::PacketType::kPlayerMove, [this, clock](const network::Packet& p) {
        auto pkt = network::PlayerMovePacket::Deserialize(p.payload);
        std::cerr << "[RPM] kPlayerMove account_id=" << pkt.account_id << " pos=(" << pkt.x << "," << pkt.y << ")" << std::endl;
        HandlePlayerMove(pkt, clock ? clock->GetServerTimeMs() : 0);
    });

    dispatcher.Subscribe(network::PacketType::kPlayerDirection, [this, clock](const network::Packet& p) {
        auto pkt = network::PlayerDirectionPacket::Deserialize(p.payload);
        HandlePlayerDirection(pkt, clock ? clock->GetServerTimeMs() : 0);
    });

    dispatcher.Subscribe(network::PacketType::kMapTransition, [this, clock](const network::Packet& p) {
        auto pkt = network::MapTransitionPacket::Deserialize(p.payload);
        HandleMapTransition(pkt, clock ? clock->GetServerTimeMs() : 0);
    });
}

void RemotePlayerManager::HandleSpawn(const network::PlayerSpawnPacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  RemotePlayer rp;
  rp.account_id = pkt.account_id;
  rp.character_id = pkt.character_id;
  rp.map_id = pkt.map_id;
  rp.interpolator.Snap(pkt.x, pkt.y);
  rp.current_x = pkt.x;
  rp.current_y = pkt.y;
  rp.direction = pkt.direction;
  rp.movement_state = pkt.movement_state;
  rp.last_update_time = timestamp;
  players_[pkt.account_id] = rp;
}

void RemotePlayerManager::HandleDespawn(const network::PlayerDespawnPacket& pkt) {
  std::lock_guard<std::mutex> lock(mutex_);
  players_.erase(pkt.account_id);
}

void RemotePlayerManager::HandleStateUpdate(const network::PlayerStatePacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = players_.find(pkt.account_id);
  if (it != players_.end()) {
    it->second.interpolator.SetTarget(it->second.current_x, it->second.current_y, pkt.x, pkt.y);
    it->second.direction = pkt.direction;
    it->second.movement_state = pkt.movement_state;
    it->second.last_update_time = timestamp;
  }
}

void RemotePlayerManager::HandlePlayerMove(const network::PlayerMovePacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = players_.find(pkt.account_id);
  if (it != players_.end()) {
    std::cerr << "[RPM] HandlePlayerMove FOUND id=" << pkt.account_id << " old=(" << it->second.current_x << "," << it->second.current_y << ") new=(" << pkt.x << "," << pkt.y << ")" << std::endl;
    it->second.interpolator.SetTarget(it->second.current_x, it->second.current_y, pkt.x, pkt.y);
    it->second.movement_state = pkt.movement_state;
    it->second.last_update_time = timestamp;
  } else {
    std::cerr << "[RPM] HandlePlayerMove NOT_FOUND id=" << pkt.account_id << " (players_.size=" << players_.size() << ")" << std::endl;
    for (const auto& [key, val] : players_) {
      std::cerr << "[RPM]   stored key=" << key << " account_id=" << val.account_id << std::endl;
    }
  }
}

void RemotePlayerManager::HandlePlayerDirection(const network::PlayerDirectionPacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = players_.find(pkt.account_id);
  if (it != players_.end()) {
    it->second.direction = pkt.direction;
    it->second.last_update_time = timestamp;
  }
}

void RemotePlayerManager::HandleMapTransition(const network::MapTransitionPacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = players_.find(pkt.account_id);
  if (it != players_.end()) {
    it->second.map_id = pkt.map_id;
    it->second.interpolator.Snap(pkt.x, pkt.y);
    it->second.current_x = pkt.x;
    it->second.current_y = pkt.y;
    it->second.last_update_time = timestamp;
  }
}

void RemotePlayerManager::HandleWorldSnapshot(const network::WorldSnapshotPacket& pkt, int64_t timestamp) {
  std::lock_guard<std::mutex> lock(mutex_);
  players_.clear();
  for (const auto& p : pkt.players) {
    RemotePlayer rp;
    rp.account_id = p.account_id;
    rp.character_id = p.character_id;
    rp.map_id = p.map_id;
    rp.interpolator.Snap(p.x, p.y);
    rp.current_x = p.x;
    rp.current_y = p.y;
    rp.direction = p.direction;
    rp.movement_state = p.movement_state;
    rp.last_update_time = timestamp;
    players_[p.account_id] = rp;
  }
}

void RemotePlayerManager::Update(float dt) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& [id, player] : players_) {
    player.interpolator.Update(dt, player.current_x, player.current_y);
  }
}

std::vector<RemotePlayer> RemotePlayerManager::GetPlayersOnMap(uint32_t map_id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<RemotePlayer> result;
  for (const auto& [id, player] : players_) {
    if (player.map_id == map_id) {
      result.push_back(player);
    }
  }
  return result;
}

std::vector<RemotePlayer> RemotePlayerManager::GetAllPlayers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<RemotePlayer> result;
    result.reserve(players_.size());
    for (const auto& pair : players_) {
        result.push_back(pair.second);
    }
    return result;
}

}  // namespace unboundmp::gameplay
