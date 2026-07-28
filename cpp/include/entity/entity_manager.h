#pragma once

#include <vector>
#include <optional>
#include "entity/entity_types.h"

namespace unboundmp::game_state { class GameState; }
namespace unboundmp::gameplay { class RemotePlayerManager; class FollowerManager; }

namespace unboundmp::entity {

class EntityManager {
 public:
  void Initialize(game_state::GameState* state, gameplay::RemotePlayerManager* remote_mgr, gameplay::FollowerManager* follower_mgr);
  void Update(float dt);
  
  const Entity& GetLocalPlayer() const;
  std::vector<Entity> GetEntitiesOnMap(const parser::MapLocation& map) const;
  std::optional<Entity> GetEntity(EntityId id) const;
  size_t GetEntityCount() const;

 private:
  Entity local_player_entity_{};
  game_state::GameState* state_ = nullptr;
  gameplay::RemotePlayerManager* remote_mgr_ = nullptr;
  gameplay::FollowerManager* follower_mgr_ = nullptr;
};

} // namespace unboundmp::entity
