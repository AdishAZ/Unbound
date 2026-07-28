#include "world/world_entity.h"

namespace unboundmp::server::world {

static std::atomic<uint64_t> s_next_entity_id{1};

uint64_t WorldEntity::NextEntityId() {
    return s_next_entity_id.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace unboundmp::server::world
