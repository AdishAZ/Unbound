#include "world/server_visibility_manager.h"
#include "utils/logger.h"

#include <algorithm>
#include <cmath>

namespace unboundmp::server::world {

bool ServerVisibilityManager::UpdateVisibility(const PlayerEntity& player,
                                                const WorldMap& map,
                                                std::vector<uint64_t>& out_spawned,
                                                std::vector<uint64_t>& out_despawned) {
    std::lock_guard lock(mutex_);

    auto& current_set = visibility_sets_[player.account_id];
    std::set<uint64_t> new_set;

    float r2 = visibility_radius_ * visibility_radius_;

    // Check other players on this map.
    auto players = map.GetPlayers();
    for (const auto& other : players) {
        if (!other || other->account_id == player.account_id) continue;
        if (!other->visible) continue;

        float dx = other->x - player.x;
        float dy = other->y - player.y;
        if (dx * dx + dy * dy <= r2) {
            new_set.insert(other->entity_id);
        }
    }

    // Check non-player entities on this map.
    auto entities = map.GetEntities();
    for (const auto& entity : entities) {
        if (!entity || !entity->visible) continue;

        float dx = entity->x - player.x;
        float dy = entity->y - player.y;
        if (dx * dx + dy * dy <= r2) {
            new_set.insert(entity->entity_id);
        }
    }

    bool changed = (new_set != current_set);

    if (changed) {
        // Determine spawns (newly visible).
        for (uint64_t eid : new_set) {
            if (current_set.find(eid) == current_set.end()) {
                out_spawned.push_back(eid);
                Logger::Debug("VisibilityManager: Entity " + std::to_string(eid) +
                              " spawned for account " + std::to_string(player.account_id));
            }
        }

        // Determine despawns (no longer visible).
        for (uint64_t eid : current_set) {
            if (new_set.find(eid) == new_set.end()) {
                out_despawned.push_back(eid);
                Logger::Debug("VisibilityManager: Entity " + std::to_string(eid) +
                              " despawned for account " + std::to_string(player.account_id));
            }
        }

        current_set = std::move(new_set);
    }

    return changed;
}

std::set<uint64_t> ServerVisibilityManager::GetVisibleEntities(uint64_t account_id) const {
    std::lock_guard lock(mutex_);
    auto it = visibility_sets_.find(account_id);
    if (it != visibility_sets_.end()) return it->second;
    return {};
}

void ServerVisibilityManager::SpawnEntity(uint64_t entity_id, uint64_t for_account_id) {
    std::lock_guard lock(mutex_);
    visibility_sets_[for_account_id].insert(entity_id);
}

void ServerVisibilityManager::DespawnEntity(uint64_t entity_id, uint64_t for_account_id) {
    std::lock_guard lock(mutex_);
    auto it = visibility_sets_.find(for_account_id);
    if (it != visibility_sets_.end()) {
        it->second.erase(entity_id);
    }
}

void ServerVisibilityManager::RemovePlayer(uint64_t account_id) {
    std::lock_guard lock(mutex_);
    visibility_sets_.erase(account_id);
    Logger::Debug("VisibilityManager: Cleared visibility state for account " +
                  std::to_string(account_id));
}

bool ServerVisibilityManager::CanSee(uint64_t observer_account_id, uint64_t entity_id) const {
    std::lock_guard lock(mutex_);
    auto it = visibility_sets_.find(observer_account_id);
    if (it == visibility_sets_.end()) return false;
    return it->second.count(entity_id) > 0;
}

}  // namespace unboundmp::server::world
