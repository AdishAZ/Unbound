#include "gameplay/entity_manager.h"
#include <cstdio>

namespace unboundmp::gameplay {

EntityManager::EntityManager(game_state::GameState* game_state)
    : game_state_(game_state) {}

void EntityManager::Update(int64_t frame_count) {
    if (!game_state_) return;
    
    entities_.clear();
    
    const auto& event_data = game_state_->GetEventObjects();
    
    for (const auto& ev : event_data.events) {
        RenderableEntity entity;
        entity.id = ev.id;
        entity.world_x = ev.x;
        entity.world_y = ev.y;
        entity.is_player = ev.is_player;
        entity.graphics_id = ev.graphics_id;
        entity.direction = ev.direction;
        
        entities_.push_back(entity);
        
        if (frame_count % 60 == 0) {
            printf("[Pipeline] EntityManager - ID: %d | Type: %s | World: (%d, %d) | GfxID: %d | Visible: 1 | Layer: Entities | AnimState: Idle\n",
                   entity.id, entity.is_player ? "Player" : "NPC", entity.world_x, entity.world_y, entity.graphics_id);
        }
    }
}

} // namespace unboundmp::gameplay
