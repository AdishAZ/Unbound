#pragma once

#include <cstdint>
#include <vector>

namespace unboundmp::render {

struct GbaTileset {
    uint8_t is_compressed;
    uint8_t is_secondary;
    uint16_t padding;
    uint32_t tiles_ptr;
    uint32_t palettes_ptr;
    uint32_t blocks_ptr;
    uint32_t behavior_ptr;
    uint32_t animation_ptr;
};

struct GbaMapLayout {
    uint32_t width;
    uint32_t height;
    uint32_t border_ptr;
    uint32_t map_ptr;
    uint32_t tileset_primary_ptr;
    uint32_t tileset_secondary_ptr;
};

struct GbaMapHeader {
    uint32_t map_layout_ptr;
    uint32_t events_ptr;
    uint32_t scripts_ptr;
    uint32_t connections_ptr;
    uint16_t music;
    uint16_t map_layout_id;
    uint8_t region_map_section_id;
    uint8_t cave;
    uint8_t weather;
    uint8_t map_type;
    uint16_t filler;
    uint8_t escape_rope;
    uint8_t flags;
    uint8_t battle_type;
};

struct GbaMetatile {
    uint16_t layer_bottom[4];
    uint16_t layer_top[4];
};

struct GbaMapConnection {
    uint8_t direction;
    int32_t offset;
    uint8_t map_group;
    uint8_t map_num;
    uint16_t filler;
};

struct GbaMapConnections {
    int32_t count;
    uint32_t connections_array_ptr;
};

} // namespace unboundmp::render
