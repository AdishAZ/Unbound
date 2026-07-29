#include "gameplay/sync_scheduler.h"
#include <iostream>

namespace unboundmp::gameplay {

SyncScheduler::SyncScheduler(game_state::GameState* state, StateChangeCallback callback)
    : state_(state), callback_(std::move(callback)) {}

void SyncScheduler::Tick() {
  if (!state_ || !state_->IsValid()) return;

  const auto& local_player = state_->GetLocalPlayer();

  uint32_t current_map = (local_player.map.bank << 16) | local_player.map.number;
  float current_x = static_cast<float>(local_player.position.x);
  float current_y = static_cast<float>(local_player.position.y);
  uint8_t current_dir = parser::ToNetworkDirection(local_player.facing);
  uint8_t current_state = parser::ToNetworkMovementState(local_player.movement);

  // Debug: log position periodically
  static int sync_debug_counter = 0;
  if (++sync_debug_counter % 60 == 0) {
    std::cerr << "[SyncScheduler] GBA pos=(" << current_x << "," << current_y 
              << ") map=" << current_map << " dir=" << (int)current_dir << std::endl;
  }

  bool map_changed = (current_map != last_map_id_);
  bool pos_changed = (current_x != last_x_ || current_y != last_y_ || current_dir != last_direction_ || current_state != last_movement_state_);

  if (map_changed) {
    network::MapTransitionPacket map_pkt;
    map_pkt.map_id = current_map;
    map_pkt.x = current_x;
    map_pkt.y = current_y;

    network::Packet p;
    p.type = network::PacketType::kMapTransition;
    p.payload = map_pkt.Serialize();
    callback_(p);
    
    last_map_id_ = current_map;
  }

  if (pos_changed || map_changed) {
    network::MoveRequestPacket req;
    req.x = current_x;
    req.y = current_y;
    req.direction = current_dir;
    req.movement_state = current_state;

    network::Packet p;
    p.type = network::PacketType::kMoveRequest;
    p.payload = req.Serialize();
    callback_(p);

    last_x_ = current_x;
    last_y_ = current_y;
    last_direction_ = current_dir;
    last_movement_state_ = current_state;
  }
}

}  // namespace unboundmp::gameplay
