// Standalone smoke test for the Milestone 11 interaction-detection layer.
// Feeds InteractionManager a scripted sequence of PlayerStateUpdate
// packets - exactly the shape NetworkManager::Poll() would hand a real
// dispatcher, same as follower_manager_example.cpp - and exercises facing
// detection, distance checks, candidate listing, and target
// selection/cycling. No emulator, no network socket, no rendering.
#include <iostream>

#include "interaction/interaction_manager.h"
#include "interaction/player_selector.h"

namespace {

using unboundmp::protocol::PlayerStateUpdate;

PlayerStateUpdate MakeState(uint32_t player_id, int32_t x, int32_t y, unboundmp::protocol::Direction facing) {
  PlayerStateUpdate state;
  state.set_player_id(player_id);
  state.set_map_bank(1);
  state.set_map_number(1);
  state.set_x(x);
  state.set_y(y);
  state.set_facing(facing);
  state.set_movement(unboundmp::protocol::MOVEMENT_MODE_WALK);
  state.set_is_moving(false);
  return state;
}

}  // namespace

int main() {
  using namespace unboundmp;

  interaction::InteractionManager manager;
  interaction::PlayerSelector selector;

  constexpr uint32_t kLocal = 1;
  constexpr uint32_t kFaced = 2;    // directly in front of kLocal
  constexpr uint32_t kNearby = 3;   // in range, but not faced
  constexpr uint32_t kFarAway = 4;  // out of range entirely

  // Local player stands at (10, 10) facing right (east).
  manager.OnPlayerStateUpdate(MakeState(kLocal, 10, 10, protocol::DIRECTION_RIGHT));
  // kFaced is exactly one tile east - directly faced.
  manager.OnPlayerStateUpdate(MakeState(kFaced, 11, 10, protocol::DIRECTION_LEFT));
  // kNearby is two tiles south - in range (distance 2), but not faced.
  manager.OnPlayerStateUpdate(MakeState(kNearby, 10, 12, protocol::DIRECTION_UP));
  // kFarAway is 20 tiles away - outside the default candidate radius.
  manager.OnPlayerStateUpdate(MakeState(kFarAway, 30, 10, protocol::DIRECTION_LEFT));

  const std::vector<interaction::InteractionCandidate> candidates = manager.GetCandidates(kLocal);
  std::cout << "Candidates for player " << kLocal << ": " << candidates.size() << " (expected 2)\n";
  for (const auto& candidate : candidates) {
    std::cout << "  player_id=" << candidate.player_id << " distance=" << candidate.distance
              << " is_faced=" << candidate.is_faced << "\n";
  }

  if (auto resolved = selector.ResolveTarget(kLocal, manager)) {
    std::cout << "ResolveTarget: " << *resolved << " (expected " << kFaced << " - facing wins over distance)\n";
  }

  // Local player turns away - nobody is faced anymore, so ResolveTarget()
  // should fall back to the nearest remaining candidate.
  manager.OnPlayerStateUpdate(MakeState(kLocal, 10, 10, protocol::DIRECTION_DOWN));
  if (auto resolved = selector.ResolveTarget(kLocal, manager)) {
    std::cout << "After turning away, ResolveTarget: " << *resolved << " (expected " << kFaced
              << " - still nearest even though no longer faced)\n";
  }

  // Manual cycling: first call should land on the nearest candidate, and
  // a second call should advance to the next one.
  auto first = selector.CycleTarget(kLocal, manager);
  auto second = selector.CycleTarget(kLocal, manager);
  std::cout << "CycleTarget sequence: " << (first ? *first : 0) << " -> " << (second ? *second : 0)
            << " (expected two distinct candidates)\n";

  manager.RemovePlayer(kFarAway);
  std::cout << "After RemovePlayer(kFarAway), snapshot present = " << manager.GetSnapshot(kFarAway).has_value()
            << " (expected 0)\n";

  return 0;
}
