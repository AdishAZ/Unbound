#pragma once

#include <cstdint>

// Screen/window resolution primitives shared by the viewport, camera, and
// scaling logic in this directory. Nothing here talks to SDL2, OpenGL, or
// an actual window - it's pure geometry, consumed by a future windowing/
// rendering milestone the same way memory/position_reader.h's Position is
// consumed by gameplay/follower_manager.h today: a data type + math the
// eventual renderer needs, with none of the renderer itself.

namespace unboundmp::render {

struct Resolution {
  int32_t width = 0;
  int32_t height = 0;

  double AspectRatioValue() const {
    return height == 0 ? 0.0 : static_cast<double>(width) / static_cast<double>(height);
  }
};

// The GBA's native framebuffer: 240x160 pixels, a 3:2 aspect ratio. This is
// a hardware fact (the same kind of fact memory/gba_memory_regions.h
// documents for memory - true of every GBA game, not something specific to
// Unbound) - the console renders into exactly this many pixels before any
// scaling is applied, and mGBA's framebuffer will always hand back exactly
// this size regardless of what window it's eventually drawn into.
inline constexpr Resolution kGbaNativeResolution{240, 160};

// The overworld tile size used by every Generation III Pokemon engine
// (Ruby/Sapphire/Emerald/FireRed/LeafGreen and engine-compatible ROM hacks,
// Unbound included): 16x16 pixels per tile. This is an engine convention
// shared across the whole Gen III codebase family, not a per-game fact that
// needs reverse engineering - unlike the RAM addresses in
// memory/address_table.h, which vary per build and are never assumed.
inline constexpr int32_t kTilePixelSize = 16;

// Native GBA screen expressed in tiles: 15 columns x 10 rows. Both divide
// evenly (240/16, 160/16), which is why the Gen III camera can center the
// player on a whole tile in the first place.
inline constexpr int32_t kNativeTileColumns = kGbaNativeResolution.width / kTilePixelSize;   // 15
inline constexpr int32_t kNativeTileRows = kGbaNativeResolution.height / kTilePixelSize;      // 10

// A common desktop 16:9 target. Not load-bearing anywhere in this module -
// every computation here takes the actual window size as input - just a
// reasonable default for examples and tests.
inline constexpr Resolution kDefault16x9Resolution{1280, 720};

}  // namespace unboundmp::render
