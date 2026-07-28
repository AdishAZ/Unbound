#pragma once

#include <cstdint>
#include <algorithm>

#include "render/aspect_ratio.h"
#include "render/resolution.h"

namespace unboundmp::render {

// The tile-space rectangle of the map that should be visible around the
// player, expressed as a column/row count plus the player's tile offset
// within it. Kept in tile units (not pixels) because that's what a future
// tile-streaming/VRAM-reading renderer needs to know which extra tiles to
// fetch and draw - this struct is the *decision* of how much world to
// show, not the pixels themselves (see overlay_layout.h for the tile ->
// pixel projection once a viewport is involved).
struct CameraWindow {
  int32_t columns = kNativeTileColumns;
  int32_t rows = kNativeTileRows;
  // Player's column/row index within the window (0-indexed). Kept
  // explicit rather than always-exactly-centered because an even number
  // of extra columns can't split perfectly around a single tile - see
  // Camera::Recompute for how the remaining tile is assigned.
  int32_t player_column = kNativeTileColumns / 2;
  int32_t player_row = kNativeTileRows / 2;

  int32_t TilesLeftOfPlayer() const { return player_column; }
  int32_t TilesRightOfPlayer() const { return columns - player_column - 1; }
};

struct CameraConfig {
  // When false, the camera never expands past the native 15x10 tile
  // window regardless of aspect ratio - widescreen falls back to plain
  // pillarboxing (DynamicViewport handles that on its own, independent of
  // this flag). Lets camera expansion be toggled off (e.g. a "classic
  // 4:3" display option, or a competitive-fairness setting so no player
  // sees more of the map than another) without touching any other layer.
  bool allow_expansion = true;

  // Camera expansion only ever adds tile *columns*, never rows, so the
  // vertical field of view - and anything that was previously off the top
  // or bottom edge of the native 240x160 frame - is identical to
  // unmodified Unbound. Only the sides open up. This cap bounds how far
  // that can go (e.g. for an ultrawide monitor) independent of whatever
  // the aspect ratio math would otherwise compute.
  int32_t max_extra_columns = 10;
};

// Computes how many tiles wide the camera should show, given the current
// viewport aspect ratio. Pure tile-space math - no pixels, no window, no
// knowledge of the player's actual map position beyond the optional
// map-edge clamp in RecomputeClamped().
class Camera {
 public:
  explicit Camera(CameraConfig config = {}) : config_(config) {}

  // `content_aspect_ratio` is normally the DynamicViewport's window aspect
  // ratio (see viewport.h) - the camera widens to match whatever the
  // player can actually see, not a hardcoded 16:9, so it behaves
  // sensibly at 16:10, 21:9, or any other window shape too.
  const CameraWindow& Recompute(AspectRatio content_aspect_ratio);

  // Same as Recompute, but clamps the result so it never requests a tile
  // column that doesn't exist, given the player currently standing at
  // absolute tile column `player_tile_x` on a map that is
  // `map_width_tiles` columns wide. Shrinks left/right independently
  // (rather than re-centering the player) so a widescreen camera near a
  // map edge stays correctly anchored on the player instead of appearing
  // to "slide" - an off-center player reads as a bug, an asymmetric
  // camera window near a map boundary does not.
  const CameraWindow& RecomputeClamped(AspectRatio content_aspect_ratio, int32_t player_tile_x,
                                        int32_t map_width_tiles);

  const CameraWindow& Window() const { return window_; }
  const CameraConfig& Config() const { return config_; }

  void SetPosition(float x, float y) {
      world_x_ = x;
      world_y_ = y;
  }
  float GetX() const { return world_x_; }
  float GetY() const { return world_y_; }
  
  void SetZoom(float zoom) { zoom_ = std::max(0.1f, zoom); }
  float GetZoom() const { return zoom_; }
  
  // Converts a world coordinate into screen pixel coordinates relative to the viewport.
  // Assumes each tile is 16x16 game pixels, and the camera is centered on (world_x_, world_y_).
  void WorldToScreen(float x, float y, int32_t viewport_content_width, int32_t viewport_content_height, float& screen_x, float& screen_y) const;
  
  // Converts screen pixel coordinates relative to the viewport into world coordinates.
  void ScreenToWorld(float screen_x, float screen_y, int32_t viewport_content_width, int32_t viewport_content_height, float& x, float& y) const;

 private:
  int32_t ExtraColumnsFor(AspectRatio content_aspect_ratio) const;

  CameraConfig config_;
  CameraWindow window_{};
  float world_x_ = 0.0f;
  float world_y_ = 0.0f;
  float zoom_ = 1.0f;
};

}  // namespace unboundmp::render
