#pragma once

#include <cstdint>

#include "gameplay/follower_animation.h"
#include "packets.pb.h"

namespace unboundmp::gameplay {

// Everything about one player's follower Pokemon needed to draw it:
// which species (and shiny/visibility), which tile it's walking toward,
// how far through that move it is (for sub-tile interpolation), which way
// it's facing, and which walk-cycle frame to show. Deliberately contains
// no rendering/sprite-sheet details (pixel dimensions, texture handles) -
// this struct is the boundary between this milestone's game-logic layer
// and the rendering milestone that turns it into pixels.
struct FollowerVisualState {
  uint32_t player_id = 0;

  uint32_t species_id = 0;
  bool visible = false;
  bool shiny = false;

  // Tile the follower is currently walking toward (or standing on, if not
  // moving). previous_tile_x/y is where it's walking *from* - a renderer
  // lerps between the two by step_progress for smooth motion.
  int32_t tile_x = 0;
  int32_t tile_y = 0;
  int32_t previous_tile_x = 0;
  int32_t previous_tile_y = 0;

  protocol::Direction facing = protocol::DIRECTION_UNSPECIFIED;

  float step_progress = 0.0f;
  bool is_moving = false;
  WalkFrame frame = WalkFrame::kStand;
};

}  // namespace unboundmp::gameplay
