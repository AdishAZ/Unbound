#include "gameplay/gameplay_sync_manager.h"

namespace unboundmp::gameplay {

GameplaySyncManager::GameplaySyncManager(std::shared_ptr<server::PlayerRegistry> registry,
                                         std::shared_ptr<server::SessionManager> session_manager)
    : registry_(registry),
      session_manager_(session_manager),
      world_sync_(std::make_unique<WorldSync>(registry)) {}

GameplaySyncManager::GameplaySyncManager(game_state::GameState* state,
                                         std::shared_ptr<RemotePlayerManager> remote_player_manager)
    : state_(state),
      remote_player_manager_(std::move(remote_player_manager)) {
  // SyncScheduler requires a callback to send packets, but we don't have the connection here.
  // We leave sync_scheduler_ null and let the client initialize it when ready.
}

void GameplaySyncManager::RegisterServerHandlers(network::PacketDispatcher& dispatcher) {
  // 1. Handle Map Transitions
  dispatcher.RegisterHandler(network::PacketType::kMapTransition, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto session = session_manager_->GetSessionByToken(p.session_token);
    if (!session) return;
    
    auto pkt = network::MapTransitionPacket::Deserialize(p.payload);
    uint64_t account_id = session->account.id;
    pkt.account_id = account_id;
    
    // First, tell old observers this player despawned (if they were on a map previously)
    auto player_opt = registry_->GetPlayer(account_id);
    if (player_opt.has_value() && player_opt->map_id != pkt.map_id) {
      VisibilityManager vis(registry_);
      auto old_observers = vis.GetObserversFor(account_id, 20.0f);
      network::PlayerDespawnPacket despawn_pkt;
      despawn_pkt.account_id = account_id;
      
      network::Packet d_p;
      d_p.type = network::PacketType::kPlayerDespawn;
      d_p.payload = despawn_pkt.Serialize();
      
      for (const auto& obs : old_observers) {
        auto obs_session = session_manager_->GetSessionByToken(obs.session_token);
        if (obs_session && obs_session->connection) {
          obs_session->connection->SendPacket(d_p);
        }
      }
    }
    
    // Update server state
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    registry_->UpdateSpatialState(account_id, pkt.map_id, pkt.x, pkt.y, 0, 0, now);
    
    // Send snapshot to the transitioning player
    auto snapshot_pkt = world_sync_->CreateSnapshotForMap(pkt.map_id);
    network::Packet snap_p;
    snap_p.type = network::PacketType::kWorldSnapshot;
    snap_p.payload = snapshot_pkt.Serialize();
    conn->SendPacket(snap_p);
    
    // Tell new observers this player spawned
    VisibilityManager vis(registry_);
    auto new_observers = vis.GetObserversFor(account_id, 20.0f);
    
    network::PlayerSpawnPacket spawn_pkt;
    spawn_pkt.account_id = account_id;
    spawn_pkt.character_id = 0;
    spawn_pkt.map_id = pkt.map_id;
    spawn_pkt.x = pkt.x;
    spawn_pkt.y = pkt.y;
    spawn_pkt.direction = 0;
    spawn_pkt.movement_state = 0;
    
    network::Packet sp_p;
    sp_p.type = network::PacketType::kPlayerSpawn;
    sp_p.payload = spawn_pkt.Serialize();
    
    for (const auto& obs : new_observers) {
      auto obs_session = session_manager_->GetSessionByToken(obs.session_token);
      if (obs_session && obs_session->connection) {
        obs_session->connection->SendPacket(sp_p);
      }
    }
  });
  
  // 2. Handle Player State (Movement)
  dispatcher.RegisterHandler(network::PacketType::kPlayerState, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto session = session_manager_->GetSessionByToken(p.session_token);
    if (!session) return;
    
    auto pkt = network::PlayerStatePacket::Deserialize(p.payload);
    uint64_t account_id = session->account.id;
    pkt.account_id = account_id;
    
    if (world_sync_->ValidateMovement(account_id, pkt)) {
      VisibilityManager vis(registry_);
      auto observers = vis.GetObserversFor(account_id, 20.0f);
      
      network::Packet out_p;
      out_p.type = network::PacketType::kPlayerState;
      out_p.payload = pkt.Serialize();
      
      for (const auto& obs : observers) {
        auto obs_session = session_manager_->GetSessionByToken(obs.session_token);
        if (obs_session && obs_session->connection) {
          obs_session->connection->SendPacket(out_p);
        }
      }
    }
  });
}

void GameplaySyncManager::RegisterClientHandlers(network::PacketDispatcher& dispatcher) {
  dispatcher.RegisterHandler(network::PacketType::kPlayerSpawn, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto pkt = network::PlayerSpawnPacket::Deserialize(p.payload);
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    remote_player_manager_->HandleSpawn(pkt, now);
  });
  
  dispatcher.RegisterHandler(network::PacketType::kPlayerDespawn, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto pkt = network::PlayerDespawnPacket::Deserialize(p.payload);
    remote_player_manager_->HandleDespawn(pkt);
  });
  
  dispatcher.RegisterHandler(network::PacketType::kPlayerState, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto pkt = network::PlayerStatePacket::Deserialize(p.payload);
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    remote_player_manager_->HandleStateUpdate(pkt, now);
  });
  
  dispatcher.RegisterHandler(network::PacketType::kWorldSnapshot, [this](network::Connection::Pointer conn, const network::Packet& p) {
    auto pkt = network::WorldSnapshotPacket::Deserialize(p.payload);
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    remote_player_manager_->HandleWorldSnapshot(pkt, now);
  });
}

}  // namespace unboundmp::gameplay
