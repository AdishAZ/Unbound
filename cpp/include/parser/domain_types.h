#pragma once
#include <cstdint>
#include <vector>

namespace unboundmp::parser {

struct PlayerPosition {
    int32_t x;
    int32_t y;
    bool operator==(const PlayerPosition& other) const {
        return x == other.x && y == other.y;
    }
};

struct MapLocation {
    uint32_t bank;
    uint32_t number;
    uint32_t CombinedId() const { return (bank << 16) | number; }
    bool operator==(const MapLocation& other) const {
        return bank == other.bank && number == other.number;
    }
};

enum class FacingDirection {
    kDown = 1,
    kUp = 2,
    kLeft = 3,
    kRight = 4
};

enum class MovementMode {
    kWalk = 1,
    kRun = 2,
    kBike = 3,
    kSurf = 4
};

struct PlayerMovementState {
    MovementMode mode;
    bool is_moving;
    bool operator==(const PlayerMovementState& other) const {
        return mode == other.mode && is_moving == other.is_moving;
    }
};

struct FollowerInfo {
    uint16_t species_id;
    bool visible;
    bool shiny;
    bool operator==(const FollowerInfo& other) const {
        return species_id == other.species_id && visible == other.visible && shiny == other.shiny;
    }
};

struct ParsedPokemonData {
    uint16_t species_id;
    uint16_t current_hp;
    uint16_t max_hp;
};

struct RawPartyData {
    std::vector<std::vector<uint8_t>> slots;
    std::vector<ParsedPokemonData> parsed_slots;
    uint32_t count;
};

struct LocalPlayerSnapshot {
    MapLocation map;
    PlayerPosition position;
    FacingDirection facing;
    PlayerMovementState movement;
};

struct EventObjectSnapshot {
    int id;
    int16_t x;
    int16_t y;
    bool is_player;
    uint8_t sprite_id;
    uint8_t graphics_id;
    uint8_t direction;
    // We can add animation state later
    
    bool operator==(const EventObjectSnapshot& other) const {
        return id == other.id && x == other.x && y == other.y && 
               is_player == other.is_player && sprite_id == other.sprite_id &&
               graphics_id == other.graphics_id && direction == other.direction;
    }
};

struct RawEventObjectData {
    std::vector<EventObjectSnapshot> events;
};

uint8_t ToNetworkDirection(FacingDirection dir);
FacingDirection FromNetworkDirection(uint8_t dir);
uint8_t ToNetworkMovementState(const PlayerMovementState& state);
PlayerMovementState FromNetworkMovementState(uint8_t state);

} // namespace unboundmp::parser
