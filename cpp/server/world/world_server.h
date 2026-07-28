#pragma once

#include "world/map_manager.h"
#include "world/player_manager.h"
#include "world/server_visibility_manager.h"

#include <cstdint>
#include <string>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// WorldServer — the central coordinator for the persistent game world.
//
// Owns MapManager, PlayerManager, and ServerVisibilityManager. Provides
// high-level APIs that orchestrate across all three managers so callers
// (e.g. packet handlers in server/main.cpp) don't need to coordinate
// managers themselves.
// ---------------------------------------------------------------------------

struct VisibilityEvent {
    uint64_t observer_account_id;
    std::shared_ptr<WorldEntity> entity;
    bool is_spawn;
};

class WorldServer {
 public:
    WorldServer() = default;
    ~WorldServer() = default;

    // --- Lifecycle ---
    void Initialize();
    void Shutdown();

    // --- Player operations (high-level) ---

    // Register a player into the world after character selection.
    // Creates a PlayerEntity, places it on the given map, and initializes
    // visibility.
    bool RegisterPlayer(uint64_t account_id,
                        uint64_t character_id,
                        const std::string& session_token,
                        const std::string& character_name,
                        uint32_t map_id,
                        float x, float y);

    // Remove a player from the world (disconnect / logout).
    void RemovePlayer(uint64_t account_id);

    // Transfer a player from their current map to a new map.
    bool TransferPlayer(uint64_t account_id,
                        uint32_t new_map_id,
                        float x, float y);

    // Handle a movement request from a client.
    bool ProcessMovementRequest(uint64_t account_id, float x, float y, uint8_t direction, uint8_t movement_state);

    // --- Periodic update ---
    // Called every server tick. Refreshes visibility for all players and returns visibility changes.
    std::vector<VisibilityEvent> Update();

    // --- Manager access ---
    MapManager& GetMapManager() { return map_manager_; }
    PlayerManager& GetPlayerManager() { return player_manager_; }
    ServerVisibilityManager& GetVisibilityManager() { return visibility_manager_; }

    const MapManager& GetMapManager() const { return map_manager_; }
    const PlayerManager& GetPlayerManager() const { return player_manager_; }
    const ServerVisibilityManager& GetVisibilityManager() const { return visibility_manager_; }

 private:
    MapManager map_manager_;
    PlayerManager player_manager_;
    ServerVisibilityManager visibility_manager_;

    bool initialized_ = false;
};

}  // namespace unboundmp::server::world
