// Standalone smoke test for the Android-support touch input layer.
// Feeds a scripted sequence of normalized touch events - exactly the shape
// the JNI bridge (android/app/src/main/cpp) would translate a real
// MotionEvent into - and prints the resulting held-button mask. No
// Android SDK/NDK, no real touchscreen, no emulator: this only proves the
// region-hit-testing / multi-finger / drag-between-regions logic behaves
// correctly on its own, same pattern as follower_manager_example.cpp.
#include <iostream>

#include "input/touch_input_mapper.h"

namespace {

void PrintHeld(const unboundmp::emulator::InputState& state) {
  using unboundmp::emulator::GbaButton;
  const std::pair<GbaButton, const char*> buttons[] = {
      {GbaButton::kA, "A"},         {GbaButton::kB, "B"},     {GbaButton::kUp, "Up"},
      {GbaButton::kDown, "Down"},   {GbaButton::kLeft, "Left"}, {GbaButton::kRight, "Right"},
      {GbaButton::kStart, "Start"}, {GbaButton::kSelect, "Select"},
      {GbaButton::kL, "L"},         {GbaButton::kR, "R"},
  };
  std::cout << "  held: [";
  bool first = true;
  for (const auto& [button, name] : buttons) {
    if (state.IsHeld(button)) {
      if (!first) std::cout << " ";
      std::cout << name;
      first = false;
    }
  }
  std::cout << "]\n";
}

}  // namespace

int main() {
  using namespace unboundmp::input;

  TouchInputMapper mapper;

  // Finger 0 presses the d-pad's Right arm.
  auto state = mapper.OnTouchEvent({{0, 0.30f, 0.72f}, TouchPhase::kDown});
  std::cout << "Finger 0 down on d-pad Right:\n";
  PrintHeld(state);

  // Finger 1 presses A at the same time - both should be held together.
  state = mapper.OnTouchEvent({{1, 0.90f, 0.72f}, TouchPhase::kDown});
  std::cout << "Finger 1 down on A (while finger 0 still holds Right):\n";
  PrintHeld(state);
  if (!state.IsHeld(unboundmp::emulator::GbaButton::kRight) ||
      !state.IsHeld(unboundmp::emulator::GbaButton::kA)) {
    std::cout << "BUG: expected both Right and A held simultaneously\n";
    return 1;
  }

  // Finger 0 drags from Right to Up without lifting - Right should release
  // and Up should become held, while finger 1's A is untouched.
  state = mapper.OnTouchEvent({{0, 0.19f, 0.63f}, TouchPhase::kMove});
  std::cout << "Finger 0 drags from Right to Up:\n";
  PrintHeld(state);
  if (state.IsHeld(unboundmp::emulator::GbaButton::kRight)) {
    std::cout << "BUG: Right should have released after drag\n";
    return 1;
  }
  if (!state.IsHeld(unboundmp::emulator::GbaButton::kUp)) {
    std::cout << "BUG: Up should be held after drag\n";
    return 1;
  }
  if (!state.IsHeld(unboundmp::emulator::GbaButton::kA)) {
    std::cout << "BUG: finger 1's A should be unaffected by finger 0's drag\n";
    return 1;
  }

  // Finger 1 lifts - only A releases, Up (finger 0) stays held.
  state = mapper.OnTouchEvent({{1, 0.90f, 0.72f}, TouchPhase::kUp});
  std::cout << "Finger 1 lifts off A:\n";
  PrintHeld(state);
  if (state.IsHeld(unboundmp::emulator::GbaButton::kA)) {
    std::cout << "BUG: A should have released\n";
    return 1;
  }
  if (!state.IsHeld(unboundmp::emulator::GbaButton::kUp)) {
    std::cout << "BUG: Up should still be held (finger 0 never lifted)\n";
    return 1;
  }

  // Reset clears everything, as if the OS interrupted the touch stream.
  mapper.Reset();
  std::cout << "After Reset:\n";
  PrintHeld(mapper.CurrentState());
  if (mapper.CurrentState().held_mask != 0) {
    std::cout << "BUG: Reset should clear all held buttons\n";
    return 1;
  }

  std::cout << "All touch_input checks passed.\n";
  return 0;
}
