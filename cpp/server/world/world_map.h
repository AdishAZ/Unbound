#pragma once

#include "world/world_entity.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// WorldMap — a loaded map instance containing players and entities.
//
// Thread-safe: uses a shared_mutex so multiple readers (e.g. visibility
// queries) can proceed concurrently while writes (add/remove) are exclusive.
// ---------------------------------------------------------------------------

class WorldMap {
 public:
    WorldMap(uint32_t map_id, const std::string& map_name);
    ~WorldMap() = default;

    // --- Identity ---
    uint32_t GetMapId() const { return metadata_.map_id; }
    const std::string& GetMapName() const { return metadata_.map_name; }
    const MapMetadata& GetMetadata() const { return metadata_; }
    void SetMetadata(const MapMetadata& metadata);

    // --- Players ---
    void AddPlayer(std::shared_ptr<PlayerEntity> player);
    bool RemovePlayer(uint64_t account_id);
    std::shared_ptr<PlayerEntity> FindPlayer(uint64_t account_id) const;
    std::vector<std::shared_ptr<PlayerEntity>> GetPlayers() const;
    size_t GetPlayerCount() const;

    // --- Generic Entities ---
    void AddEntity(std::shared_ptr<WorldEntity> entity);
    bool RemoveEntity(uint64_t entity_id);
    std::shared_ptr<WorldEntity> FindEntity(uint64_t entity_id) const;
    std::vector<std::shared_ptr<WorldEntity>> GetEntities() const;
    size_t GetEntityCount() const;

    // --- Spawn Points ---
    void AddSpawnPoint(const SpawnPoint& spawn);
    std::optional<SpawnPoint> GetDefaultSpawn() const;
    const std::vector<SpawnPoint>& GetSpawnPoints() const;

 private:
    MapMetadata metadata_;

    mutable std::shared_mutex mutex_;

    // Players keyed by account_id for O(1) lookup.
    std::unordered_map<uint64_t, std::shared_ptr<PlayerEntity>> players_;

    // All non-player entities keyed by entity_id.
    std::unordered_map<uint64_t, std::shared_ptr<WorldEntity>> entities_;

    std::vector<SpawnPoint> spawn_points_;
};

}  // namespace unboundmp::server::world
