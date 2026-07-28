#pragma once

#include "world/world_entity.h"
#include "world/world_map.h"

#include <cmath>
#include <cstdint>
#include <mutex>
#include <set>
#include <unordered_map>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// ServerVisibilityManager — determines which entities each player can see.
//
// Named "Server" to avoid collision with the client-side VisibilityManager
// in include/gameplay/visibility_manager.h.
//
// Maintains per-player visibility sets. When UpdateVisibility() is called
// for a player, it recomputes the set of nearby entity IDs and tracks
// which entities have newly entered or left view (spawn/despawn events).
// ---------------------------------------------------------------------------

class ServerVisibilityManager {
 public:
    ServerVisibilityManager() = default;
    ~ServerVisibilityManager() = default;

    // --- Configuration ---
    void SetVisibilityRadius(float radius) { visibility_radius_ = radius; }
    float GetVisibilityRadius() const { return visibility_radius_; }

    // --- Core API ---

    // Recompute the visibility set for `player` on `map`.
    // Populates `out_spawned` and `out_despawned` with entity IDs that entered/left view.
    // Returns true if the set changed (spawns or despawns occurred).
    bool UpdateVisibility(const PlayerEntity& player, const WorldMap& map,
                          std::vector<uint64_t>& out_spawned,
                          std::vector<uint64_t>& out_despawned);

    // Get the current visibility set for a player (entity IDs they can see).
    std::set<uint64_t> GetVisibleEntities(uint64_t account_id) const;

    // Explicitly mark an entity as visible/invisible for a player.
    void SpawnEntity(uint64_t entity_id, uint64_t for_account_id);
    void DespawnEntity(uint64_t entity_id, uint64_t for_account_id);

    // Remove all visibility state for a player (on disconnect).
    void RemovePlayer(uint64_t account_id);

    // --- Query helpers ---
    bool CanSee(uint64_t observer_account_id, uint64_t entity_id) const;

 private:
    float visibility_radius_ = 20.0f;

    mutable std::mutex mutex_;

    // account_id → set of entity_ids currently visible to that player.
    std::unordered_map<uint64_t, std::set<uint64_t>> visibility_sets_;
};

}  // namespace unboundmp::server::world
