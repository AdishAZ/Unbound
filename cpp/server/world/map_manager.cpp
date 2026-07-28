#include "world/map_manager.h"
#include "utils/logger.h"

namespace unboundmp::server::world {

WorldMap* MapManager::LoadMap(uint32_t map_id, const std::string& map_name) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it != maps_.end()) {
        // Already loaded — return the existing instance.
        return it->second.get();
    }

    auto map = std::make_unique<WorldMap>(map_id, map_name);

    // Add a default spawn point at the origin.
    SpawnPoint default_spawn;
    default_spawn.x = 0.0f;
    default_spawn.y = 0.0f;
    default_spawn.direction = 0;
    default_spawn.name = "default";
    default_spawn.is_default = true;
    map->AddSpawnPoint(default_spawn);

    WorldMap* ptr = map.get();
    maps_[map_id] = std::move(map);

    Logger::Info("MapManager: Loaded map " + std::to_string(map_id) + " (" + map_name + ")");
    return ptr;
}

bool MapManager::UnloadMap(uint32_t map_id) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it == maps_.end()) return false;

    Logger::Info("MapManager: Unloaded map " + std::to_string(map_id) +
                 " (" + it->second->GetMapName() + ")");
    maps_.erase(it);
    return true;
}

WorldMap* MapManager::GetMap(uint32_t map_id) const {
    std::lock_guard lock(mutex_);
    auto it = maps_.find(map_id);
    if (it != maps_.end()) return it->second.get();
    return nullptr;
}

bool MapManager::IsMapLoaded(uint32_t map_id) const {
    std::lock_guard lock(mutex_);
    return maps_.count(map_id) > 0;
}

std::vector<uint32_t> MapManager::GetLoadedMapIds() const {
    std::lock_guard lock(mutex_);
    std::vector<uint32_t> ids;
    ids.reserve(maps_.size());
    for (const auto& [id, _] : maps_) {
        ids.push_back(id);
    }
    return ids;
}

size_t MapManager::GetLoadedMapCount() const {
    std::lock_guard lock(mutex_);
    return maps_.size();
}

// ---------------------------------------------------------------------------
// Player routing
// ---------------------------------------------------------------------------

bool MapManager::PlayerEnterMap(uint32_t map_id, std::shared_ptr<PlayerEntity> player) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it == maps_.end()) return false;

    it->second->AddPlayer(std::move(player));
    Logger::Info("MapManager: Player (account " +
                 std::to_string(player ? player->account_id : 0) +
                 ") entered map " + std::to_string(map_id));
    return true;
}

bool MapManager::PlayerLeaveMap(uint32_t map_id, uint64_t account_id) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it == maps_.end()) return false;

    bool removed = it->second->RemovePlayer(account_id);
    if (removed) {
        Logger::Info("MapManager: Player (account " +
                     std::to_string(account_id) +
                     ") left map " + std::to_string(map_id));
    }
    return removed;
}

// ---------------------------------------------------------------------------
// Entity routing
// ---------------------------------------------------------------------------

bool MapManager::RegisterEntity(uint32_t map_id, std::shared_ptr<WorldEntity> entity) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it == maps_.end()) return false;

    Logger::Debug("MapManager: Registered entity " +
                  std::to_string(entity->entity_id) +
                  " on map " + std::to_string(map_id));
    it->second->AddEntity(std::move(entity));
    return true;
}

bool MapManager::RemoveEntity(uint32_t map_id, uint64_t entity_id) {
    std::lock_guard lock(mutex_);

    auto it = maps_.find(map_id);
    if (it == maps_.end()) return false;

    bool removed = it->second->RemoveEntity(entity_id);
    if (removed) {
        Logger::Debug("MapManager: Removed entity " +
                      std::to_string(entity_id) +
                      " from map " + std::to_string(map_id));
    }
    return removed;
}

// ---------------------------------------------------------------------------
// Broadcast
// ---------------------------------------------------------------------------

void MapManager::BroadcastToMap(
    uint32_t map_id,
    const std::function<void(const PlayerEntity&)>& callback) const {

    std::lock_guard lock(mutex_);
    auto it = maps_.find(map_id);
    if (it == maps_.end()) return;

    auto players = it->second->GetPlayers();
    for (const auto& player : players) {
        if (player) callback(*player);
    }
}

}  // namespace unboundmp::server::world
