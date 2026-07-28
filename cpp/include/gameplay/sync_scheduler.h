#pragma once

#include <memory>
#include <functional>
#include "network/packet.h"
#include "game_state/game_state.h"

namespace unboundmp::gameplay {

// Responsible for polling the local emulator memory and generating sync packets
class SyncScheduler {
 public:
  // Callback when a state change is detected that should be sent to the server.
  using StateChangeCallback = std::function<void(const network::Packet&)>;

  explicit SyncScheduler(game_state::GameState* state, StateChangeCallback callback);

  // Called periodically (e.g., from the FrameCallback or a timer thread)
  // Polls the memory and fires callback if state changed.
  void Tick();

 private:
  game_state::GameState* state_;
  StateChangeCallback callback_;

  // Last known states for delta compression
  uint32_t last_map_id_ = 0xFFFFFFFF;
  float last_x_ = -1.0f;
  float last_y_ = -1.0f;
  uint8_t last_direction_ = 255;
  uint8_t last_movement_state_ = 255;
};

}  // namespace unboundmp::gameplay
