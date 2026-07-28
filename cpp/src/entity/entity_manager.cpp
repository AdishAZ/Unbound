#include "entity/entity_manager.h"
#include "game_state/game_state.h"
#include "gameplay/remote_player_manager.h"
#include "gameplay/follower_manager.h"

namespace unboundmp::entity {

void EntityManager::Initialize(game_state::GameState* state, gameplay::RemotePlayerManager* remote_mgr, gameplay::FollowerManager* follower_mgr) {
    state_ = state;
    remote_mgr_ = remote_mgr;
    follower_mgr_ = follower_mgr;
    
    local_player_entity_.id = kLocalPlayerEntityId;
    local_player_entity_.type = EntityType::kLocalPlayer;
}

void EntityManager::Update(float /*dt*/) {
    if (!state_ || !state_->IsValid()) return;
    
    const auto& snapshot = state_->GetLocalPlayer();
    local_player_entity_.map = snapshot.map;
    local_player_entity_.x = static_cast<float>(snapshot.position.x);
    local_player_entity_.y = static_cast<float>(snapshot.position.y);
    local_player_entity_.facing = snapshot.facing;
    local_player_entity_.movement = snapshot.movement.mode;
    local_player_entity_.visible = true;
    local_player_entity_.last_update_time = state_->GetFrameCount();
}

const Entity& EntityManager::GetLocalPlayer() const {
    return local_player_entity_;
}

std::vector<Entity> EntityManager::GetEntitiesOnMap(const parser::MapLocation& map) const {
    std::vector<Entity> result;
    
    if (local_player_entity_.map == map && state_ && state_->IsValid()) {
        result.push_back(local_player_entity_);
    }
    
    if (remote_mgr_) {
        auto remote_players = remote_mgr_->GetPlayersOnMap(map.CombinedId());
        for (const auto& rp : remote_players) {
            Entity ent;
            ent.id = rp.account_id;
            ent.type = EntityType::kRemotePlayer;
            ent.map = map;
            ent.x = rp.current_x;
            ent.y = rp.current_y;
            ent.facing = static_cast<parser::FacingDirection>(rp.direction);
            ent.movement = parser::MovementMode::kWalk; 
            ent.visible = true;
            ent.last_update_time = rp.last_update_time;
            result.push_back(ent);
        }
    }
    
    if (follower_mgr_) {
        size_t current_count = result.size();
        for (size_t i = 0; i < current_count; ++i) {
            auto owner_id = result[i].id;
            auto visual_state = follower_mgr_->GetVisualState(static_cast<uint32_t>(owner_id));
            if (visual_state && visual_state->visible) {
                Entity follower;
                follower.id = owner_id + 0x100000000ULL;
                follower.type = EntityType::kFollower;
                follower.map = map;
                follower.x = static_cast<float>(visual_state->tile_x) + visual_state->step_progress * (visual_state->tile_x - visual_state->previous_tile_x);
                follower.y = static_cast<float>(visual_state->tile_y) + visual_state->step_progress * (visual_state->tile_y - visual_state->previous_tile_y);
                follower.facing = static_cast<parser::FacingDirection>(visual_state->facing);
                follower.movement = parser::MovementMode::kWalk;
                follower.visible = visual_state->visible;
                follower.last_update_time = result[i].last_update_time;
                result.push_back(follower);
            }
        }
    }
    
    return result;
}

std::optional<Entity> EntityManager::GetEntity(EntityId id) const {
    if (id == kLocalPlayerEntityId) {
        return local_player_entity_;
    }
    return std::nullopt;
}

size_t EntityManager::GetEntityCount() const {
    size_t count = 1;
    if (remote_mgr_) {
        count += remote_mgr_->GetPlayerCount();
        if (follower_mgr_) {
            count += follower_mgr_->AllVisualStates().size();
        }
    }
    return count;
}

} // namespace unboundmp::entity
