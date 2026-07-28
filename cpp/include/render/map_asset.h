#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "render/gba_map_types.h"
#include "render/texture_atlas.h"

namespace unboundmp::render {

// Represents a fully parsed and ready-to-render map, independent of GBA hardware formats.
struct MapAsset {
    int width = 0;
    int height = 0;
    
    int border_width = 2;
    int border_height = 2;
    
    // The tile grid (width * height). Each element is a metatile ID.
    std::vector<uint16_t> map_blocks;
    
    // The border grid (border_width * border_height)
    std::vector<uint16_t> border_blocks;
    
    // The metatile definitions (primary + secondary combined).
    // The engine uses a unified index for metatiles.
    std::vector<GbaMetatile> metatiles;
    
    // The decoded graphical data (1024 tiles * 16 palettes, RGBA pixels)
    std::vector<uint32_t> decoded_rgba;
    int tile_count = 0;
    
    // Validates the map bounds and returns the metatile ID, or fallback border tile
    uint16_t GetMetatileId(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return map_blocks[y * width + x] & 0x3FF; // Mask out collision bits for rendering
        }
        
        // If outside map, wrap using the border blocks
        // In GBA maps, the border is a 2x2 grid that repeats infinitely
        int bx = ((x % border_width) + border_width) % border_width;
        int by = ((y % border_height) + border_height) % border_height;
        return border_blocks[by * border_width + bx] & 0x3FF;
    }
};

} // namespace unboundmp::render
