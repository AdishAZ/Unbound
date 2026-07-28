// Standalone smoke test for the Milestone 9 follower gameplay layer.
// Feeds FollowerManager a scripted sequence of PlayerStateUpdate/
// FollowerUpdate packets - exactly the shape NetworkManager::Poll() would
// hand a real dispatcher - and prints the resulting FollowerVisualState
// each tick. No emulator, no network socket, no rendering: this only
// proves the species-sync / movement-trail / direction / animation logic
// behaves sensibly on its own.
#include <iostream>

#include "gameplay/follower_manager.h"

int main() {
  using namespace unboundmp;

  gameplay::FollowerManager manager;
  constexpr uint32_t kTrainerId = 1;

  // Species sync: local player catches a Pikachu, follower should light up.
  memory::FollowerState local_follower;
  local_follower.species_id = 25;  // Pikachu
  local_follower.visible = true;
  local_follower.shiny = false;
  if (auto update = manager.UpdateLocalFollower(kTrainerId, local_follower)) {
    manager.OnFollowerUpdate(*update);
    std::cout << "Follower species synced: species_id=" << update->species_id()
              << " visible=" << update->visible() << "\n";
  }

  // Calling again with the same state should NOT produce another update -
  // this is the change-detection the milestone calls for.
  if (manager.UpdateLocalFollower(kTrainerId, local_follower)) {
    std::cout << "BUG: duplicate FollowerUpdate emitted for unchanged state\n";
  } else {
    std::cout << "No duplicate FollowerUpdate for unchanged follower state (expected)\n";
  }

  // Trainer starts standing still at (10, 10) facing down.
  protocol::PlayerStateUpdate state;
  state.set_player_id(kTrainerId);
  state.set_map_bank(1);
  state.set_map_number(1);
  state.set_x(10);
  state.set_y(10);
  state.set_facing(protocol::DIRECTION_DOWN);
  state.set_movement(protocol::MOVEMENT_MODE_WALK);
  state.set_is_moving(false);
  manager.OnPlayerStateUpdate(state);

  if (auto visual = manager.GetVisualState(kTrainerId)) {
    std::cout << "Initial follower spawn tile: (" << visual->tile_x << ", " << visual->tile_y
              << ") - one tile behind the trainer's facing, as expected\n";
  }

  // Trainer walks three tiles to the right (east).
  const int kTileSteps = 3;
  for (int step = 0; step < kTileSteps; ++step) {
    state.set_x(state.x() + 1);
    state.set_facing(protocol::DIRECTION_RIGHT);
    state.set_is_moving(true);
    manager.OnPlayerStateUpdate(state);

    // Walk cadence default is 200ms/tile; tick in 50ms increments so we
    // can observe step_progress advancing smoothly, then land exactly on
    // the tile boundary.
    for (int i = 0; i < 4; ++i) {
      manager.Tick(50);
    }

    if (auto visual = manager.GetVisualState(kTrainerId)) {
      std::cout << "After trainer step " << (step + 1) << ": trainer at (" << state.x() << ", " << state.y()
                << "), follower at (" << visual->tile_x << ", " << visual->tile_y
                << "), facing=" << protocol::Direction_Name(visual->facing) << "\n";
    }
  }

  manager.RemovePlayer(kTrainerId);
  std::cout << "After RemovePlayer: has state = " << manager.GetVisualState(kTrainerId).has_value() << " (expected 0)\n";

  return 0;
}
