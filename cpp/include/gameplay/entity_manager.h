#pragma once

#include <memory>
#include <vector>
#include "game_state/game_state.h"

namespace unboundmp::gameplay {

struct RenderableEntity {
    int id;
    int world_x;
    int world_y;
    bool is_player;
    uint8_t graphics_id;
    uint8_t direction;
};

class EntityManager {
public:
    EntityManager(game_state::GameState* game_state);
    
    void Update(int64_t frame_count);
    
    const std::vector<RenderableEntity>& GetEntities() const { return entities_; }

private:
    game_state::GameState* game_state_;
    std::vector<RenderableEntity> entities_;
};

} // namespace unboundmp::gameplay
