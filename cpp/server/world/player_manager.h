#pragma once

#include "world/world_entity.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// PlayerManager — owns all online PlayerEntity instances.
//
// This is distinct from PlayerRegistry (which tracks connection/persistence
// state). PlayerManager tracks *world* state — which map a player is on,
// their position, direction, and movement mode.
// ---------------------------------------------------------------------------

class PlayerManager {
 public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    // --- Registration ---
    std::shared_ptr<PlayerEntity> RegisterPlayer(
        uint64_t account_id,
        uint64_t character_id,
        const std::string& session_token,
        const std::string& character_name = "");

    bool RemovePlayer(uint64_t account_id);

    // --- Lookups ---
    std::shared_ptr<PlayerEntity> FindPlayer(uint64_t account_id) const;
    std::shared_ptr<PlayerEntity> FindByCharacter(uint64_t character_id) const;
    std::shared_ptr<PlayerEntity> FindBySession(const std::string& session_token) const;

    std::vector<std::shared_ptr<PlayerEntity>> GetAllPlayers() const;
    size_t GetPlayerCount() const;

 private:
    mutable std::mutex mutex_;

    // Primary index: account_id → PlayerEntity.
    std::unordered_map<uint64_t, std::shared_ptr<PlayerEntity>> players_by_account_;

    // Secondary indices for O(1) lookup by character or session.
    std::unordered_map<uint64_t, uint64_t> account_by_character_;
    std::unordered_map<std::string, uint64_t> account_by_session_;
};

}  // namespace unboundmp::server::world
