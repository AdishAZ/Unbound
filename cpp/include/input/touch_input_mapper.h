#pragma once

#include <optional>
#include <string>
#include <vector>

#include "emulator/emulator_core.h"

namespace unboundmp::input {

// A finger touching the screen. Coordinates are normalized [0, 1] against
// the current window/surface size, not raw pixels - this keeps the mapper
// itself independent of any actual window size, DPI, or platform (it is
// exercised by desktop unit tests/examples exactly like every other
// "foundation" layer in this project, e.g. render/, and only wired to real
// Android touch events by the JNI bridge in android/app/src/main/cpp).
struct TouchPoint {
  int id = 0;  // platform touch/pointer id, for tracking across move events
  float x = 0.0f;
  float y = 0.0f;
};

enum class TouchPhase {
  kDown,
  kMove,
  kUp,
  kCancel,
};

struct TouchEvent {
  TouchPoint point;
  TouchPhase phase = TouchPhase::kDown;
};

// A single on-screen control region. Rectangles are normalized [0, 1] the
// same as TouchPoint, so the same layout works at any resolution/aspect
// ratio - the render milestone's DynamicViewport (render/viewport.h)
// already establishes this normalized-then-scaled pattern for on-screen
// geometry, and this reuses it for input regions instead of duplicating a
// second coordinate system.
struct TouchRegion {
  emulator::GbaButton button;
  float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;

  bool Contains(float px, float py) const {
    return px >= x && px <= x + width && py >= y && py <= y + height;
  }
};

// The default on-screen control layout: a four-way d-pad in the lower
// left, A/B face buttons in the lower right, Start/Select as small pads
// along the bottom center, and L/R shoulder strips along the top edge.
// Returns normalized regions; a real UI can render this same layout, or a
// player can remap individual regions (TouchInputMapper doesn't care where
// the regions came from).
std::vector<TouchRegion> DefaultLayout();

// Turns a stream of raw touch events into GBA button state, handling
// multi-touch (each finger tracked independently by id) and touch-drag
// (a finger that lands on the d-pad and slides to a different direction
// updates which button is held, matching how real GBA d-pad touch overlays
// behave - the original press position doesn't "stick" the finger to one
// button once it starts moving).
class TouchInputMapper {
 public:
  explicit TouchInputMapper(std::vector<TouchRegion> layout = DefaultLayout());

  // Feeds one touch event and returns the resulting full held-button
  // state. Safe to call once per touch event as they arrive from the
  // platform (Android's MotionEvent, or a desktop mouse-as-touch stand-in
  // for testing).
  emulator::InputState OnTouchEvent(const TouchEvent& event);

  // Current held-button state without feeding a new event (e.g. for a
  // per-frame poll instead of an event callback).
  emulator::InputState CurrentState() const { return state_; }

  void SetLayout(std::vector<TouchRegion> layout) { layout_ = std::move(layout); }
  const std::vector<TouchRegion>& layout() const { return layout_; }

  // Clears all tracked fingers and held buttons (e.g. on app pause / a
  // touch stream being interrupted by an OS gesture).
  void Reset();

 private:
  std::vector<TouchRegion> layout_;
  emulator::InputState state_;

  // Which button (if any) each active finger id is currently holding, so a
  // finger lift only releases the button *that finger* is responsible for
  // - important once two fingers are down at once (e.g. B + Right).
  struct TrackedFinger {
    int id;
    std::optional<emulator::GbaButton> held_button;
  };
  std::vector<TrackedFinger> fingers_;

  std::optional<emulator::GbaButton> RegionAt(float x, float y) const;
  void RecomputeHeldMaskFromFingers();
};

}  // namespace unboundmp::input
