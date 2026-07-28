#pragma once

#include <cstdint>
#include <string>
#include <atomic>

namespace unboundmp::server::world {

// ---------------------------------------------------------------------------
// Entity Types
// ---------------------------------------------------------------------------

enum class EntityType : uint8_t {
    kPlayer = 0,
    kNPC    = 1,
    kObject = 2,
    kWarp   = 3
};

// ---------------------------------------------------------------------------
// WorldEntity — base class for all runtime entities in the game world.
// ---------------------------------------------------------------------------

struct WorldEntity {
    uint64_t entity_id = 0;
    uint32_t map_id    = 0;
    float x            = 0.0f;
    float y            = 0.0f;
    uint8_t direction  = 0;
    bool visible       = true;
    EntityType type    = EntityType::kObject;

    virtual ~WorldEntity() = default;

    // Generate a globally unique entity ID.
    static uint64_t NextEntityId();
};

// ---------------------------------------------------------------------------
// PlayerEntity — a logged-in player present in the world.
// ---------------------------------------------------------------------------

struct PlayerEntity : WorldEntity {
    uint64_t account_id    = 0;
    uint64_t character_id  = 0;
    std::string session_token;
    std::string character_name;
    uint8_t movement_state = 0;

    PlayerEntity() { type = EntityType::kPlayer; }
};

// ---------------------------------------------------------------------------
// NPCEntity — a server-managed non-player character.
// ---------------------------------------------------------------------------

struct NPCEntity : WorldEntity {
    uint32_t npc_id    = 0;
    uint32_t script_id = 0;
    std::string name;

    NPCEntity() { type = EntityType::kNPC; }
};

// ---------------------------------------------------------------------------
// ObjectEntity — an interactive object (item ball, sign, etc.).
// ---------------------------------------------------------------------------

struct ObjectEntity : WorldEntity {
    uint32_t object_id   = 0;
    std::string object_type;
    bool interactable    = true;

    ObjectEntity() { type = EntityType::kObject; }
};

// ---------------------------------------------------------------------------
// WarpEntity — a map transition trigger.
// ---------------------------------------------------------------------------

struct WarpEntity : WorldEntity {
    uint32_t target_map_id = 0;
    float target_x         = 0.0f;
    float target_y         = 0.0f;

    WarpEntity() { type = EntityType::kWarp; }
};

// ---------------------------------------------------------------------------
// SpawnPoint — a named location where players/NPCs can appear.
// ---------------------------------------------------------------------------

struct SpawnPoint {
    float x = 0.0f;
    float y = 0.0f;
    uint8_t direction = 0;
    std::string name;
    bool is_default = false;
};

// ---------------------------------------------------------------------------
// MapMetadata — static information about a map.
// ---------------------------------------------------------------------------

struct MapMetadata {
    uint32_t map_id = 0;
    std::string map_name;
    uint32_t width  = 0;
    uint32_t height = 0;
    std::string region;
};

}  // namespace unboundmp::server::world
