#pragma once

#include "world/world_map.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// MapManager — owns all loaded WorldMap instances.
//
// Maps are loaded dynamically and can be unloaded when empty. The manager
// provides player enter/leave helpers that coordinate with the WorldMap
// directly.
// ---------------------------------------------------------------------------

class MapManager {
 public:
    MapManager() = default;
    ~MapManager() = default;

    // --- Map lifecycle ---
    WorldMap* LoadMap(uint32_t map_id, const std::string& map_name);
    bool UnloadMap(uint32_t map_id);
    WorldMap* GetMap(uint32_t map_id) const;
    bool IsMapLoaded(uint32_t map_id) const;
    std::vector<uint32_t> GetLoadedMapIds() const;
    size_t GetLoadedMapCount() const;

    // --- Player routing ---
    bool PlayerEnterMap(uint32_t map_id, std::shared_ptr<PlayerEntity> player);
    bool PlayerLeaveMap(uint32_t map_id, uint64_t account_id);

    // --- Entity routing ---
    bool RegisterEntity(uint32_t map_id, std::shared_ptr<WorldEntity> entity);
    bool RemoveEntity(uint32_t map_id, uint64_t entity_id);

    // --- Broadcast ---
    // Calls `callback` for every player on the given map.
    void BroadcastToMap(uint32_t map_id,
                        const std::function<void(const PlayerEntity&)>& callback) const;

 private:
    mutable std::mutex mutex_;
    std::unordered_map<uint32_t, std::unique_ptr<WorldMap>> maps_;
};

}  // namespace unboundmp::server::world
