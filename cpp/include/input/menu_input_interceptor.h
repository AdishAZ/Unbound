#pragma once
#include "emulator/emulator_core.h"
#include <SDL2/SDL.h>

namespace unboundmp::input {

class MenuInputInterceptor {
public:
    MenuInputInterceptor() = default;

    // Filters the emulator input. If `is_overlay_open` is true, it clears all input
    // so gameplay is paused. If it detects a start press, it intercepts it.
    void FilterInput(emulator::InputState& input, bool is_overlay_open);
    
    // Returns true if the menu should be opened this frame, and consumes the request.
    bool ConsumeMenuOpenRequest();

    // Triggered externally (e.g. by the Bag UI button)
    void RequestMenuOpen() { menu_open_requested_ = true; }

private:
    bool start_was_held_ = false;
    bool menu_open_requested_ = false;
};

} // namespace unboundmp::input
