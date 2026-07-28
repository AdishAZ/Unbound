#pragma once

#include <cstdint>
#include <optional>

#include "memory/gba_memory_regions.h"
#include "memory/memory_api.h"

namespace unboundmp::memory {

// Decoded GBA OAM sprite attributes. Unlike the other readers in this
// directory, this one needs NO reverse-engineered/Unbound-specific
// addresses at all - OAM lives at a fixed hardware address (see
// gba_memory_regions.h) and its attribute bit layout is documented GBA
// hardware behavior (GBATek "OBJ Attributes"), identical for every GBA
// game. This reader tells you "what's on screen at OAM slot N" (position,
// tile, size, flip, priority) - it does NOT tell you which OAM slot
// corresponds to "the player" or "the follower Pokemon"; that mapping is
// game-specific and would need to be reverse engineered (e.g. by watching
// which slot's x/y tracks the known player position from position_reader)
// before this reader is useful for "find the player's sprite".
enum class SpriteShape : uint8_t { kSquare = 0, kWide = 1, kTall = 2, kReserved = 3 };

struct SpriteAttributes {
  int32_t y = 0;   // 0-255, wraps
  int32_t x = 0;   // 0-511 (9-bit, sign-relevant for affine), wraps
  bool disabled = false;
  bool double_size_or_affine = false;
  SpriteShape shape = SpriteShape::kSquare;
  uint8_t size_param = 0;  // combine with `shape` per GBATek's size/shape table for pixel dims
  bool horizontal_flip = false;
  bool vertical_flip = false;
  uint16_t tile_index = 0;
  uint8_t priority = 0;
  uint8_t palette_index = 0;  // only meaningful in 16-color (4bpp) mode
  bool use_256_color_palette = false;
};

class SpriteReader {
 public:
  explicit SpriteReader(const MemoryApi& memory) : memory_(memory) {}

  // `oam_index` must be in [0, gba::kOamEntryCount). Returns std::nullopt
  // for an out-of-range index; every in-range index always has *some*
  // attributes (possibly a disabled/blank sprite) since OAM is fixed-size
  // hardware memory, not a variable-length game structure.
  std::optional<SpriteAttributes> Read(uint32_t oam_index) const {
    if (oam_index >= gba::kOamEntryCount) {
      return std::nullopt;
    }

    const uint32_t entry_base = gba::kOamBase + oam_index * gba::kOamEntryStrideBytes;
    const uint16_t attr0 = memory_.ReadU16(entry_base + 0);
    const uint16_t attr1 = memory_.ReadU16(entry_base + 2);
    const uint16_t attr2 = memory_.ReadU16(entry_base + 4);

    SpriteAttributes out;

    // attr0: bits 0-7 Y, bits 8-9 obj mode/affine flags, bit 9 or 8
    // double-size/disable (mode-dependent - simplified here to the common
    // "disable when not affine" bit 9), bits 10-11 gfx mode, bit 12 mosaic,
    // bit 13 color mode (0=16/16, 1=256/1), bits 14-15 shape.
    out.y = attr0 & 0x00FF;
    const bool affine = (attr0 & 0x0100) != 0;
    const bool disable_or_double = (attr0 & 0x0200) != 0;
    out.double_size_or_affine = affine;
    out.disabled = !affine && disable_or_double;
    out.use_256_color_palette = (attr0 & 0x2000) != 0;
    out.shape = static_cast<SpriteShape>((attr0 >> 14) & 0x3);

    // attr1: bits 0-8 X (9-bit), bits 9-13 affine index or flip flags
    // (non-affine: bit 12 hflip, bit 13 vflip), bits 14-15 size.
    out.x = attr1 & 0x01FF;
    if (!affine) {
      out.horizontal_flip = (attr1 & 0x1000) != 0;
      out.vertical_flip = (attr1 & 0x2000) != 0;
    }
    out.size_param = static_cast<uint8_t>((attr1 >> 14) & 0x3);

    // attr2: bits 0-9 tile index, bits 10-11 priority, bits 12-15 palette
    // (only used in 4bpp/16-color mode).
    out.tile_index = attr2 & 0x03FF;
    out.priority = static_cast<uint8_t>((attr2 >> 10) & 0x3);
    out.palette_index = static_cast<uint8_t>((attr2 >> 12) & 0xF);

    return out;
  }

 private:
  const MemoryApi& memory_;
};

}  // namespace unboundmp::memory
