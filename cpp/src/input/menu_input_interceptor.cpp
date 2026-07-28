#include "input/menu_input_interceptor.h"

namespace unboundmp::input {

void MenuInputInterceptor::FilterInput(emulator::InputState& input, bool is_overlay_open) {
    if (is_overlay_open) {
        input.Clear();
        return;
    }
    
    bool start_is_held = input.IsHeld(emulator::GbaButton::kStart);
    
    if (start_is_held) {
        if (!start_was_held_) {
            menu_open_requested_ = true;
        }
        input.Release(emulator::GbaButton::kStart);
    }
    
    start_was_held_ = start_is_held;
}

bool MenuInputInterceptor::ConsumeMenuOpenRequest() {
    bool requested = menu_open_requested_;
    menu_open_requested_ = false;
    return requested;
}

} // namespace unboundmp::input
