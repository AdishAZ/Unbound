#pragma once

#include <cstdint>

#include "render/camera.h"
#include "render/viewport.h"

namespace unboundmp::render {

// A point in window pixel coordinates, already accounting for the
// viewport's letterbox offset (see DynamicViewport::Rect) - (0, 0) here is
// the top-left corner of the window, not the top-left corner of the game
// content.
struct ScreenPoint {
  int32_t x = 0;
  int32_t y = 0;
};

// Fixed screen-space anchors for HUD elements (e.g. a connection-status
// icon, a nearby-player-count badge) that are pinned to a corner/edge of
// the *game content* rather than to a world tile. These need to move
// whenever DynamicViewport recomputes: a widescreen window's game content
// is wider than the native 240x160 frame, so an overlay anchored at "the
// top-right corner" must resolve against the live viewport rect, not a
// hardcoded 240x160 assumption left over from a 4:3-only build.
enum class OverlayAnchor {
  kTopLeft,
  kTopCenter,
  kTopRight,
  kBottomLeft,
  kBottomCenter,
  kBottomRight,
  kCenter,
};

struct OverlayLayoutConfig {
  // Inset from the resolved anchor edge, in *content* pixels (GBA-native
  // pixel units) rather than window pixels, then scaled by the viewport -
  // this keeps the margin looking visually consistent (e.g. "4 native
  // pixels off the edge") regardless of window size, instead of a fixed
  // window-pixel margin that would look enormous at 1x and tiny at 6x.
  int32_t margin_px = 4;
};

// Resolves where on screen fixed HUD anchors and per-player world markers
// should be drawn, given the current viewport/camera state. Pure
// coordinate math - no actual drawing, no font/sprite handling; those
// belong to the future SDL2/OpenGL rendering milestone this is foundation
// for (see README.md's Milestone status section).
class OverlayLayout {
 public:
  explicit OverlayLayout(OverlayLayoutConfig config = {}) : config_(config) {}

  // Resolves a fixed HUD anchor to a window pixel coordinate for the
  // viewport's *current* size. Call again after every
  // DynamicViewport::Resize().
  ScreenPoint Resolve(OverlayAnchor anchor, const DynamicViewport& viewport) const;

  // Projects a tile-space offset from the local player's camera tile
  // (e.g. a remote player standing 3 tiles right, 1 tile up) into a
  // window pixel coordinate, given the current camera window and
  // viewport. This is what a future rendering milestone uses to place
  // another player's sprite or name-tag - the math already accounts for
  // camera expansion, so a remote player near the newly-visible edge of a
  // widescreen camera lands in the correct place instead of the
  // pre-widescreen, native-only position it would have had before this
  // milestone.
  ScreenPoint ProjectTile(int32_t tile_dx_from_local_player, int32_t tile_dy_from_local_player,
                           const Camera& camera, const DynamicViewport& viewport) const;

 private:
  OverlayLayoutConfig config_;
};

}  // namespace unboundmp::render
