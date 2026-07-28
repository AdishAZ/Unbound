#pragma once

#include <cstdint>

namespace unboundmp::parser {

// GBA maps include a 7-tile padding in memory for camera bounds handling.
// CoordinateNormalizer strips this hardware-specific quirk to provide
// pure world-space logical coordinates to the rest of the engine.
class CoordinateNormalizer {
public:
    static constexpr int kMapPaddingTiles = 7;

    static void Normalize(int16_t& x, int16_t& y) {
        int16_t old_x = x;
        int16_t old_y = y;
        x -= kMapPaddingTiles;
        y -= kMapPaddingTiles;
        // Commenting out printf to avoid spam during rendering,
        // it should only be logged during parsing
    }

    static void Normalize(uint16_t& x, uint16_t& y) {
        x = static_cast<uint16_t>(static_cast<int16_t>(x) - kMapPaddingTiles);
        y = static_cast<uint16_t>(static_cast<int16_t>(y) - kMapPaddingTiles);
    }
    
    static void Normalize(int& x, int& y) {
        x -= kMapPaddingTiles;
        y -= kMapPaddingTiles;
    }
};

} // namespace unboundmp::parser
