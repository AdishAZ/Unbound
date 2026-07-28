#include "render/camera.h"

#include <algorithm>
#include <cmath>

namespace unboundmp::render {

int32_t Camera::ExtraColumnsFor(AspectRatio content_aspect_ratio) const {
  if (!config_.allow_expansion || !content_aspect_ratio.WantsCameraExpansion()) {
    return 0;
  }

  // Solve for the column count whose columns:rows ratio matches the
  // target aspect ratio, holding rows fixed at the native 10 (tiles are
  // square, so columns:rows in tile units is also the pixel aspect
  // ratio) - see CameraConfig::max_extra_columns for why rows never
  // change.
  const double target_columns = content_aspect_ratio.Value() * static_cast<double>(kNativeTileRows);
  const auto wanted = static_cast<int32_t>(std::lround(target_columns)) - kNativeTileColumns;
  return std::clamp(wanted, 0, config_.max_extra_columns);
}

const CameraWindow& Camera::Recompute(AspectRatio content_aspect_ratio) {
  const int32_t extra = ExtraColumnsFor(content_aspect_ratio);

  // Split extra columns as evenly as possible; an odd leftover column goes
  // on the right, matching how the native 15-column window already breaks
  // its own center-of-odd-width tie (7 tiles left of the player, 7 right,
  // player in column index 7 of 0-14).
  const int32_t left_extra = extra / 2;

  window_.columns = kNativeTileColumns + extra;
  window_.rows = kNativeTileRows;
  window_.player_column = kNativeTileColumns / 2 + left_extra;
  window_.player_row = kNativeTileRows / 2;
  return window_;
}

const CameraWindow& Camera::RecomputeClamped(AspectRatio content_aspect_ratio, int32_t player_tile_x,
                                              int32_t map_width_tiles) {
  Recompute(content_aspect_ratio);
  if (map_width_tiles <= 0) {
    return window_;
  }

  const int32_t desired_left = window_.TilesLeftOfPlayer();
  const int32_t desired_right = window_.TilesRightOfPlayer();

  const int32_t max_left = std::max(player_tile_x, 0);
  const int32_t max_right = std::max(map_width_tiles - 1 - player_tile_x, 0);

  const int32_t left = std::min(desired_left, max_left);
  const int32_t right = std::min(desired_right, max_right);

  window_.player_column = left;
  window_.columns = left + 1 + right;
  return window_;
}



void Camera::WorldToScreen(float x, float y, int32_t viewport_content_width, int32_t viewport_content_height, float& screen_x, float& screen_y) const {
    // 1 tile = 16 pixels. Camera center is at world_x_, world_y_.
    // Viewport center is at viewport_content_width/2, viewport_content_height/2.
    float dx = x - world_x_;
    float dy = y - world_y_;
    
    screen_x = (viewport_content_width / 2.0f) + (dx * 16.0f * zoom_);
    screen_y = (viewport_content_height / 2.0f) + (dy * 16.0f * zoom_);
}

void Camera::ScreenToWorld(float screen_x, float screen_y, int32_t viewport_content_width, int32_t viewport_content_height, float& x, float& y) const {
    float dx = (screen_x - (viewport_content_width / 2.0f)) / (16.0f * zoom_);
    float dy = (screen_y - (viewport_content_height / 2.0f)) / (16.0f * zoom_);
    
    x = world_x_ + dx;
    y = world_y_ + dy;
}

}  // namespace unboundmp::render
