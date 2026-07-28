#pragma once

#include <cstdint>
#include "parser/domain_types.h"

namespace unboundmp::entity {

using EntityId = uint64_t;
constexpr EntityId kLocalPlayerEntityId = 0;

enum class EntityType {
  kLocalPlayer,
  kRemotePlayer,
  kFollower
};

struct Entity {
  EntityId id = 0;
  EntityType type = EntityType::kLocalPlayer;
  parser::MapLocation map{};
  float x = 0.0f;
  float y = 0.0f;
  parser::FacingDirection facing = parser::FacingDirection::kDown;
  parser::MovementMode movement = parser::MovementMode::kWalk;
  bool visible = true;
  int64_t last_update_time = 0;
};

} // namespace unboundmp::entity
