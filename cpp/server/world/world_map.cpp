#include "world/world_map.h"

namespace unboundmp::server::world {

WorldMap::WorldMap(uint32_t map_id, const std::string& map_name) {
    metadata_.map_id = map_id;
    metadata_.map_name = map_name;
}

void WorldMap::SetMetadata(const MapMetadata& metadata) {
    std::unique_lock lock(mutex_);
    metadata_ = metadata;
}

// ---------------------------------------------------------------------------
// Players
// ---------------------------------------------------------------------------

void WorldMap::AddPlayer(std::shared_ptr<PlayerEntity> player) {
    if (!player) return;
    std::unique_lock lock(mutex_);
    players_[player->account_id] = std::move(player);
}

bool WorldMap::RemovePlayer(uint64_t account_id) {
    std::unique_lock lock(mutex_);
    return players_.erase(account_id) > 0;
}

std::shared_ptr<PlayerEntity> WorldMap::FindPlayer(uint64_t account_id) const {
    std::shared_lock lock(mutex_);
    auto it = players_.find(account_id);
    if (it != players_.end()) return it->second;
    return nullptr;
}

std::vector<std::shared_ptr<PlayerEntity>> WorldMap::GetPlayers() const {
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<PlayerEntity>> result;
    result.reserve(players_.size());
    for (const auto& [id, player] : players_) {
        result.push_back(player);
    }
    return result;
}

size_t WorldMap::GetPlayerCount() const {
    std::shared_lock lock(mutex_);
    return players_.size();
}

// ---------------------------------------------------------------------------
// Generic Entities
// ---------------------------------------------------------------------------

void WorldMap::AddEntity(std::shared_ptr<WorldEntity> entity) {
    if (!entity) return;
    std::unique_lock lock(mutex_);
    entities_[entity->entity_id] = std::move(entity);
}

bool WorldMap::RemoveEntity(uint64_t entity_id) {
    std::unique_lock lock(mutex_);
    return entities_.erase(entity_id) > 0;
}

std::shared_ptr<WorldEntity> WorldMap::FindEntity(uint64_t entity_id) const {
    std::shared_lock lock(mutex_);
    auto it = entities_.find(entity_id);
    if (it != entities_.end()) return it->second;
    return nullptr;
}

std::vector<std::shared_ptr<WorldEntity>> WorldMap::GetEntities() const {
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<WorldEntity>> result;
    result.reserve(entities_.size());
    for (const auto& [id, entity] : entities_) {
        result.push_back(entity);
    }
    return result;
}

size_t WorldMap::GetEntityCount() const {
    std::shared_lock lock(mutex_);
    return entities_.size();
}

// ---------------------------------------------------------------------------
// Spawn Points
// ---------------------------------------------------------------------------

void WorldMap::AddSpawnPoint(const SpawnPoint& spawn) {
    std::unique_lock lock(mutex_);
    spawn_points_.push_back(spawn);
}

std::optional<SpawnPoint> WorldMap::GetDefaultSpawn() const {
    std::shared_lock lock(mutex_);
    for (const auto& sp : spawn_points_) {
        if (sp.is_default) return sp;
    }
    // Fallback: return the first spawn point if one exists.
    if (!spawn_points_.empty()) return spawn_points_.front();
    return std::nullopt;
}

const std::vector<SpawnPoint>& WorldMap::GetSpawnPoints() const {
    // Note: caller should hold no assumption about thread safety of the
    // returned reference. For iteration under concurrency, use GetDefaultSpawn
    // or copy the vector.
    return spawn_points_;
}

}  // namespace unboundmp::server::world
