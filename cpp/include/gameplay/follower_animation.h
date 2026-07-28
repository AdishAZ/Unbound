#pragma once

#include <cstdint>

#include "packets.pb.h"

namespace unboundmp::gameplay {

// Overworld sprites in this era of Pokemon engine use a 3-frame walk
// cycle: standing, and two alternating "foot forward" frames. This is a
// documented fact about the vanilla pokeemerald-derived sprite sheets
// (each object-event has exactly 3 movement frames per direction), not
// specific to Unbound - if Unbound's follower graphics turn out to use a
// different frame count, only the rendering milestone that maps WalkFrame
// to an actual sprite sheet cell needs to change, not this enum's meaning
// (still: idle vs first step vs second step).
enum class WalkFrame : uint8_t {
  kStand = 0,
  kStepA = 1,
  kStepB = 2,
};

// How long (in ms) a single tile-to-tile move takes at each movement
// mode. These are starting hypotheses to verify against the real game
// (see docs/REVERSE_ENGINEERING.md's approach of "confirm by observation"
// used throughout the memory/ readers), not measured facts about
// Unbound - overridable per MovementCadence instance rather than baked in
// as constants, for exactly that reason. Run/bike being faster than a walk
// and surf having its own pace are the only load-bearing assumptions;
// the exact millisecond figures are tunable.
struct MovementCadence {
  uint32_t walk_step_ms = 200;
  uint32_t run_step_ms = 100;
  uint32_t bike_step_ms = 80;
  uint32_t surf_step_ms = 150;

  uint32_t StepDurationFor(protocol::MovementMode mode) const {
    switch (mode) {
      case protocol::MOVEMENT_MODE_RUN:
        return run_step_ms;
      case protocol::MOVEMENT_MODE_BIKE:
        return bike_step_ms;
      case protocol::MOVEMENT_MODE_SURF:
        return surf_step_ms;
      case protocol::MOVEMENT_MODE_WALK:
      case protocol::MOVEMENT_MODE_UNSPECIFIED:
      default:
        return walk_step_ms;
    }
  }
};

// Result of advancing the animation by one Tick(). `step_progress` is the
// 0..1 fraction of the way through the current tile-to-tile move - a
// rendering milestone lerps between the follower's previous and target
// tile by this fraction to draw smooth sub-tile motion instead of
// snapping tile-to-tile. `completed_step` is true exactly on the Tick()
// call where progress reached 1.0, which is the signal FollowerManager
// uses to pull the next target tile off the MovementTrail.
struct AnimationTick {
  WalkFrame frame = WalkFrame::kStand;
  float step_progress = 0.0f;
  bool completed_step = false;
};

// Pure timer/frame logic - no memory or network access, so it's testable
// standalone and reusable for both the local player's own follower and
// every remote player's follower.
class FollowerAnimator {
 public:
  explicit FollowerAnimator(MovementCadence cadence = MovementCadence{}) : cadence_(cadence) {}

  // Advances the animation by `elapsed_ms`. `mode`/`is_moving` reflect the
  // trainer's current state (the follower always matches its trainer's
  // pace and mode - it never moves under its own timing). While not
  // moving, the animation holds at kStand with step_progress reset to 0
  // rather than freezing mid-stride, matching how the trainer's own sprite
  // resets to standing the instant it stops.
  AnimationTick Tick(protocol::MovementMode mode, bool is_moving, uint32_t elapsed_ms) {
    if (!is_moving) {
      elapsed_in_step_ms_ = 0;
      return AnimationTick{WalkFrame::kStand, 0.0f, false};
    }

    const uint32_t step_duration_ms = cadence_.StepDurationFor(mode);
    elapsed_in_step_ms_ += elapsed_ms;

    bool completed = false;
    // A single Tick() covering more than one full step duration (e.g. a
    // slow/laggy frame) still only reports one completed step per call -
    // FollowerManager's caller is expected to call Tick() often enough
    // (once per game frame) that this doesn't visibly skip tiles; wrapping
    // multiple times in one call would desync step_parity_ from the
    // number of tiles actually consumed off the MovementTrail.
    if (elapsed_in_step_ms_ >= step_duration_ms) {
      elapsed_in_step_ms_ -= step_duration_ms;
      if (elapsed_in_step_ms_ > step_duration_ms) {
        elapsed_in_step_ms_ = step_duration_ms;
      }
      step_parity_ = !step_parity_;
      completed = true;
    }

    AnimationTick tick;
    tick.step_progress =
        step_duration_ms == 0 ? 1.0f : static_cast<float>(elapsed_in_step_ms_) / static_cast<float>(step_duration_ms);
    tick.completed_step = completed;
    tick.frame = step_parity_ ? WalkFrame::kStepB : WalkFrame::kStepA;
    return tick;
  }

 private:
  MovementCadence cadence_;
  uint32_t elapsed_in_step_ms_ = 0;
  bool step_parity_ = false;
};

}  // namespace unboundmp::gameplay
