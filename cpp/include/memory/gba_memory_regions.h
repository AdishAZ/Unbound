#pragma once

#include <cstdint>

// GBA CPU address space regions, as documented in GBATek ("GBA Memory Map")
// and mirrored by libmgba's own address decoding (mgba/internal/gba/memory.h
// uses these same base addresses internally). These are hardware facts, not
// anything specific to Pokemon Unbound or its engine - every GBA game's
// memory looks like this. Do not add game-specific offsets here; those
// belong in memory/address_table.h and must come from actual reverse
// engineering (see docs/REVERSE_ENGINEERING.md), never guessed.

namespace unboundmp::memory::gba {

// General-purpose working RAM: 256 KiB, external to the CPU, slower than
// IWRAM. This is where most game state (player position, party data, etc.)
// tends to live in GBA Pokemon engines, but the *exact* offsets within this
// region are game-specific and unknown until reverse engineered.
inline constexpr uint32_t kEwramBase = 0x02000000;
inline constexpr uint32_t kEwramSize = 0x00040000;  // 256 KiB

// Internal working RAM: 32 KiB, on-chip, faster than EWRAM. Some engines
// keep hot-path state (e.g. current object event table) here instead.
inline constexpr uint32_t kIwramBase = 0x03000000;
inline constexpr uint32_t kIwramSize = 0x00008000;  // 32 KiB

// Memory-mapped I/O registers (PPU/APU/timers/DMA/keypad/etc. control
// registers). KEYINPUT (button state as seen by the *hardware*, distinct
// from whatever setKeys() the emulator was told) lives at kIoBase + 0x130.
inline constexpr uint32_t kIoBase = 0x04000000;
inline constexpr uint32_t kKeyInputOffset = 0x130;

// Palette RAM: 1 KiB, two 512-entry (well, 256 background + 256 sprite)
// 15-bit BGR555 palettes.
inline constexpr uint32_t kPaletteRamBase = 0x05000000;
inline constexpr uint32_t kPaletteRamSize = 0x00000400;  // 1 KiB
inline constexpr uint32_t kSpritePaletteOffset = 0x200;  // palette RAM + 0x200 = OBJ palettes

// Video RAM: 96 KiB, tile/tilemap/bitmap data.
inline constexpr uint32_t kVramBase = 0x06000000;
inline constexpr uint32_t kVramSize = 0x00018000;  // 96 KiB

// Object Attribute Memory: 1 KiB, 128 sprite entries of 8 bytes each
// (attr0/attr1/attr2, 6 bytes used + 2 bytes affine parameter interleave).
// This is exactly what sprite_reader.h reads from - see that file for the
// per-attribute bit layout (also straight from GBATek, not game-specific).
inline constexpr uint32_t kOamBase = 0x07000000;
inline constexpr uint32_t kOamSize = 0x00000400;  // 1 KiB
inline constexpr uint32_t kOamEntryCount = 128;
inline constexpr uint32_t kOamEntryStrideBytes = 8;

// Cartridge ROM, mirrored across three wait-state regions; games normally
// only reference the first (0x08000000).
inline constexpr uint32_t kRomBase = 0x08000000;

// Cartridge SRAM/backup memory (battery save), when present as memory-
// mapped SRAM rather than flash/EEPROM (Unbound's actual save type is
// determined by its ROM header / libmgba's save-type autodetection, not
// assumed here).
inline constexpr uint32_t kSramBase = 0x0E000000;

}  // namespace unboundmp::memory::gba
