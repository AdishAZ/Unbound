#include "world/world_server.h"
#include "utils/logger.h"

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void WorldServer::Initialize() {
    if (initialized_) return;

    Logger::Info("WorldServer: Initializing...");

    // Load a set of default/well-known maps.
    // In the future these could come from a config file or the database.
    // Map IDs mirror the GBA engine's (bank << 8 | number) convention, but
    // for now we use simple sequential IDs as placeholders.
    map_manager_.LoadMap(0, "Littleroot Town");
    map_manager_.LoadMap(1, "Route 101");
    map_manager_.LoadMap(2, "Oldale Town");
    map_manager_.LoadMap(3, "Route 102");
    map_manager_.LoadMap(4, "Petalburg City");

    visibility_manager_.SetVisibilityRadius(20.0f);

    initialized_ = true;
    Logger::Info("WorldServer: Initialized with " +
                 std::to_string(map_manager_.GetLoadedMapCount()) + " maps.");
}

void WorldServer::Shutdown() {
    if (!initialized_) return;

    Logger::Info("WorldServer: Shutting down...");

    // Remove all players first.
    auto all_players = player_manager_.GetAllPlayers();
    for (const auto& player : all_players) {
        RemovePlayer(player->account_id);
    }

    // Unload all maps.
    auto map_ids = map_manager_.GetLoadedMapIds();
    for (uint32_t id : map_ids) {
        map_manager_.UnloadMap(id);
    }

    initialized_ = false;
    Logger::Info("WorldServer: Shutdown complete.");
}

// ---------------------------------------------------------------------------
// Player operations
// ---------------------------------------------------------------------------

bool WorldServer::RegisterPlayer(uint64_t account_id,
                                  uint64_t character_id,
                                  const std::string& session_token,
                                  const std::string& character_name,
                                  uint32_t map_id,
                                  float x, float y) {
    // Ensure the target map is loaded (auto-load if not).
    WorldMap* map = map_manager_.GetMap(map_id);
    if (!map) {
        map = map_manager_.LoadMap(map_id, "Map_" + std::to_string(map_id));
        if (!map) {
            Logger::Error("WorldServer: Failed to load map " + std::to_string(map_id) +
                          " for player registration.");
            return false;
        }
    }

    // Create the PlayerEntity.
    auto player_entity = player_manager_.RegisterPlayer(
        account_id, character_id, session_token, character_name);
    if (!player_entity) return false;

    // Set initial world position.
    player_entity->map_id    = map_id;
    player_entity->x         = x;
    player_entity->y         = y;
    player_entity->direction = 0;

    // Place the player on the map.
    map_manager_.PlayerEnterMap(map_id, player_entity);

    // Initialize visibility.
    std::vector<uint64_t> dummy_spawn, dummy_despawn;
    visibility_manager_.UpdateVisibility(*player_entity, *map, dummy_spawn, dummy_despawn);

    Logger::Info("WorldServer: Player (account " + std::to_string(account_id) +
                 ") registered on map " + std::to_string(map_id) +
                 " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    return true;
}

void WorldServer::RemovePlayer(uint64_t account_id) {
    auto player = player_manager_.FindPlayer(account_id);
    if (!player) return;

    uint32_t map_id = player->map_id;

    // Remove from map.
    map_manager_.PlayerLeaveMap(map_id, account_id);

    // Clear visibility state.
    visibility_manager_.RemovePlayer(account_id);

    // Remove from player manager.
    player_manager_.RemovePlayer(account_id);

    Logger::Info("WorldServer: Player (account " + std::to_string(account_id) +
                 ") removed from world.");
}

bool WorldServer::TransferPlayer(uint64_t account_id,
                                  uint32_t new_map_id,
                                  float x, float y) {
    auto player = player_manager_.FindPlayer(account_id);
    if (!player) {
        Logger::Warn("WorldServer: Cannot transfer — player (account " +
                     std::to_string(account_id) + ") not found.");
        return false;
    }

    uint32_t old_map_id = player->map_id;

    // Remove from old map.
    map_manager_.PlayerLeaveMap(old_map_id, account_id);
    visibility_manager_.RemovePlayer(account_id);

    // Ensure new map is loaded.
    WorldMap* new_map = map_manager_.GetMap(new_map_id);
    if (!new_map) {
        new_map = map_manager_.LoadMap(new_map_id, "Map_" + std::to_string(new_map_id));
        if (!new_map) {
            Logger::Error("WorldServer: Failed to load destination map " +
                          std::to_string(new_map_id) + " for transfer.");
            return false;
        }
    }

    // Update player position.
    player->map_id = new_map_id;
    player->x      = x;
    player->y      = y;

    // Place on new map.
    map_manager_.PlayerEnterMap(new_map_id, player);

    // Recompute visibility on the new map.
    std::vector<uint64_t> dummy_spawn, dummy_despawn;
    visibility_manager_.UpdateVisibility(*player, *new_map, dummy_spawn, dummy_despawn);

    Logger::Info("WorldServer: Player (account " + std::to_string(account_id) +
                 ") transferred from map " + std::to_string(old_map_id) +
                 " to map " + std::to_string(new_map_id) +
                 " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    return true;
}

// ---------------------------------------------------------------------------
// Periodic update & Movement
// ---------------------------------------------------------------------------

bool WorldServer::ProcessMovementRequest(uint64_t account_id, float x, float y, uint8_t direction, uint8_t movement_state) {
    auto player = player_manager_.FindPlayer(account_id);
    if (!player) return false;

    // Movement validation could happen here (speed checks, collision bounds).
    // For now, accept all moves.
    player->x = x;
    player->y = y;
    player->direction = direction;
    player->movement_state = movement_state;

    return true;
}

std::vector<VisibilityEvent> WorldServer::Update() {
    std::vector<VisibilityEvent> events;
    if (!initialized_) return events;

    // Refresh visibility for every online player.
    auto all_players = player_manager_.GetAllPlayers();
    for (const auto& player : all_players) {
        if (!player) continue;

        WorldMap* map = map_manager_.GetMap(player->map_id);
        if (!map) continue;

        std::vector<uint64_t> spawned;
        std::vector<uint64_t> despawned;
        if (visibility_manager_.UpdateVisibility(*player, *map, spawned, despawned)) {
            for (uint64_t eid : spawned) {
                auto entity = map->FindEntity(eid); // Only works for non-players in the current implementation?
                if (!entity) {
                    // It might be a player.
                    auto players = map->GetPlayers();
                    for (const auto& p : players) {
                        if (p->entity_id == eid) {
                            entity = p;
                            break;
                        }
                    }
                }
                if (entity) {
                    events.push_back({player->account_id, entity, true});
                }
            }
            for (uint64_t eid : despawned) {
                // Despawned entity might no longer be on the map, we just need its ID.
                // But we need the entity type to know if it was a player. We will create a dummy entity with the ID just to convey the despawn event.
                // Wait, if it despawned by leaving range, it's still on the map.
                auto entity = map->FindEntity(eid);
                if (!entity) {
                    auto players = map->GetPlayers();
                    for (const auto& p : players) {
                        if (p->entity_id == eid) {
                            entity = p;
                            break;
                        }
                    }
                }
                
                if (!entity) {
                    // Fallback dummy for despawns if it completely vanished from the map
                    auto dummy = std::make_shared<WorldEntity>();
                    dummy->entity_id = eid;
                    entity = dummy; 
                }
                events.push_back({player->account_id, entity, false});
            }
        }
    }
    return events;
}

}  // namespace unboundmp::server::world
