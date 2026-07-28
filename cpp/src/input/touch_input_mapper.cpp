#include "input/touch_input_mapper.h"

#include <algorithm>

namespace unboundmp::input {

using emulator::GbaButton;
using emulator::InputState;

std::vector<TouchRegion> DefaultLayout() {
  std::vector<TouchRegion> regions;

  // D-pad: lower-left, arranged as a plus sign within a 0.30 x 0.30
  // normalized box. Each arm is its own region rather than one big diamond
  // so up+right etc. can be held simultaneously by two fingers, and so a
  // single finger's drag between arms produces clean direction changes.
  constexpr float kPadX = 0.04f, kPadY = 0.62f, kPadSize = 0.30f;
  const float third = kPadSize / 3.0f;
  regions.push_back({GbaButton::kUp, kPadX + third, kPadY, third, third});
  regions.push_back({GbaButton::kDown, kPadX + third, kPadY + 2 * third, third, third});
  regions.push_back({GbaButton::kLeft, kPadX, kPadY + third, third, third});
  regions.push_back({GbaButton::kRight, kPadX + 2 * third, kPadY + third, third, third});

  // Face buttons: lower-right, B below-left of A (matches GBA's physical
  // layout, not a generic gamepad's).
  regions.push_back({GbaButton::kA, 0.84f, 0.66f, 0.13f, 0.13f});
  regions.push_back({GbaButton::kB, 0.70f, 0.74f, 0.13f, 0.13f});

  // Start/Select: small pads, bottom center.
  regions.push_back({GbaButton::kSelect, 0.40f, 0.90f, 0.09f, 0.06f});
  regions.push_back({GbaButton::kStart, 0.51f, 0.90f, 0.09f, 0.06f});

  // Shoulder buttons: thin strips along the top edge.
  regions.push_back({GbaButton::kL, 0.0f, 0.0f, 0.20f, 0.08f});
  regions.push_back({GbaButton::kR, 0.80f, 0.0f, 0.20f, 0.08f});

  return regions;
}

TouchInputMapper::TouchInputMapper(std::vector<TouchRegion> layout) : layout_(std::move(layout)) {}

std::optional<GbaButton> TouchInputMapper::RegionAt(float x, float y) const {
  for (const auto& region : layout_) {
    if (region.Contains(x, y)) return region.button;
  }
  return std::nullopt;
}

void TouchInputMapper::RecomputeHeldMaskFromFingers() {
  state_.Clear();
  for (const auto& finger : fingers_) {
    if (finger.held_button) state_.Press(*finger.held_button);
  }
}

InputState TouchInputMapper::OnTouchEvent(const TouchEvent& event) {
  auto finger_it = std::find_if(fingers_.begin(), fingers_.end(),
                                 [&](const TrackedFinger& f) { return f.id == event.point.id; });

  switch (event.phase) {
    case TouchPhase::kDown: {
      const auto button = RegionAt(event.point.x, event.point.y);
      if (finger_it == fingers_.end()) {
        fingers_.push_back({event.point.id, button});
      } else {
        finger_it->held_button = button;
      }
      break;
    }
    case TouchPhase::kMove: {
      const auto button = RegionAt(event.point.x, event.point.y);
      if (finger_it != fingers_.end()) {
        // Drag between regions (e.g. d-pad Up -> Right) updates which
        // button this finger holds; dragging off all regions releases it.
        finger_it->held_button = button;
      } else {
        // A move event for a finger we never saw a Down for (e.g. mapper
        // was Reset() mid-touch) - treat it like a fresh Down.
        fingers_.push_back({event.point.id, button});
      }
      break;
    }
    case TouchPhase::kUp:
    case TouchPhase::kCancel: {
      if (finger_it != fingers_.end()) {
        fingers_.erase(finger_it);
      }
      break;
    }
  }

  RecomputeHeldMaskFromFingers();
  return state_;
}

void TouchInputMapper::Reset() {
  fingers_.clear();
  state_.Clear();
}

}  // namespace unboundmp::input
