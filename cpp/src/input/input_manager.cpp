#include "input/input_manager.h"
#include <iostream>

namespace unboundmp::input {

InputManager::InputManager() {
    // Initialize state
    for (int i = 0; i < static_cast<int>(GameAction::kCount); ++i) {
        GameAction action = static_cast<GameAction>(i);
        action_held_[action] = false;
        action_just_pressed_[action] = false;
        action_just_released_[action] = false;
    }
}

InputManager::~InputManager() {
    Shutdown();
}

void InputManager::Initialize() {
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    SetDefaultBindings();
}

void InputManager::Shutdown() {
    if (controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void InputManager::SetDefaultBindings() {
    SetKeyBinding(GameAction::kButtonA, SDL_SCANCODE_Z);
    SetKeyBinding(GameAction::kButtonB, SDL_SCANCODE_X);
    SetKeyBinding(GameAction::kButtonL, SDL_SCANCODE_A);
    SetKeyBinding(GameAction::kButtonR, SDL_SCANCODE_S);
    SetKeyBinding(GameAction::kButtonStart, SDL_SCANCODE_RETURN);
    SetKeyBinding(GameAction::kButtonSelect, SDL_SCANCODE_BACKSPACE);
    SetKeyBinding(GameAction::kDpadUp, SDL_SCANCODE_UP);
    SetKeyBinding(GameAction::kDpadDown, SDL_SCANCODE_DOWN);
    SetKeyBinding(GameAction::kDpadLeft, SDL_SCANCODE_LEFT);
    SetKeyBinding(GameAction::kDpadRight, SDL_SCANCODE_RIGHT);
    
    SetKeyBinding(GameAction::kSaveState, SDL_SCANCODE_F5);
    SetKeyBinding(GameAction::kLoadState, SDL_SCANCODE_F7);
    SetKeyBinding(GameAction::kFastForward, SDL_SCANCODE_SPACE);
    SetKeyBinding(GameAction::kToggleHUD, SDL_SCANCODE_F1);
    SetKeyBinding(GameAction::kToggleDevOverlay, SDL_SCANCODE_F12);
    SetKeyBinding(GameAction::kMute, SDL_SCANCODE_M);
    SetKeyBinding(GameAction::kScreenshot, SDL_SCANCODE_F11);
    
    SetKeyBinding(GameAction::kOpenSettings, SDL_SCANCODE_ESCAPE);
    SetKeyBinding(GameAction::kOpenChat, SDL_SCANCODE_T);

    // Default gamepad profile
    GamepadProfile profile;
    profile.name = "Default";
    profile.button_map[SDL_CONTROLLER_BUTTON_A] = GameAction::kButtonA;
    profile.button_map[SDL_CONTROLLER_BUTTON_B] = GameAction::kButtonB;
    profile.button_map[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = GameAction::kButtonL;
    profile.button_map[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = GameAction::kButtonR;
    profile.button_map[SDL_CONTROLLER_BUTTON_START] = GameAction::kButtonStart;
    profile.button_map[SDL_CONTROLLER_BUTTON_BACK] = GameAction::kButtonSelect;
    profile.button_map[SDL_CONTROLLER_BUTTON_DPAD_UP] = GameAction::kDpadUp;
    profile.button_map[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = GameAction::kDpadDown;
    profile.button_map[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = GameAction::kDpadLeft;
    profile.button_map[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = GameAction::kDpadRight;
    SetGamepadProfile(profile);
}

void InputManager::SetKeyBinding(GameAction action, SDL_Scancode key) {
    key_bindings_[action] = key;
    RebuildReverseLookup();
}

SDL_Scancode InputManager::GetKeyBinding(GameAction action) const {
    auto it = key_bindings_.find(action);
    if (it != key_bindings_.end()) {
        return it->second;
    }
    return SDL_SCANCODE_UNKNOWN;
}

std::string InputManager::GetKeyName(GameAction action) const {
    SDL_Scancode code = GetKeyBinding(action);
    if (code != SDL_SCANCODE_UNKNOWN) {
        return SDL_GetScancodeName(code);
    }
    return "Unbound";
}

std::string InputManager::GetActionName(GameAction action) const {
    switch (action) {
        case GameAction::kButtonA: return "A Button";
        case GameAction::kButtonB: return "B Button";
        case GameAction::kButtonL: return "L Button";
        case GameAction::kButtonR: return "R Button";
        case GameAction::kButtonStart: return "Start Button";
        case GameAction::kButtonSelect: return "Select Button";
        case GameAction::kDpadUp: return "D-Pad Up";
        case GameAction::kDpadDown: return "D-Pad Down";
        case GameAction::kDpadLeft: return "D-Pad Left";
        case GameAction::kDpadRight: return "D-Pad Right";
        case GameAction::kSaveState: return "Save State";
        case GameAction::kLoadState: return "Load State";
        case GameAction::kFastForward: return "Fast Forward";
        case GameAction::kToggleHUD: return "Toggle HUD";
        case GameAction::kToggleDevOverlay: return "Toggle Dev Overlay";
        case GameAction::kMute: return "Mute Audio";
        case GameAction::kScreenshot: return "Screenshot";
        case GameAction::kOpenSettings: return "Open Settings";
        case GameAction::kOpenChat: return "Open Chat";
        default: return "Unknown Action";
    }
}

void InputManager::RebuildReverseLookup() {
    reverse_bindings_.clear();
    for (const auto& [action, scancode] : key_bindings_) {
        reverse_bindings_[scancode].push_back(action);
    }
}

bool InputManager::HasConflict(GameAction action, SDL_Scancode key) const {
    for (const auto& [a, k] : key_bindings_) {
        if (a != action && k == key) {
            return true;
        }
    }
    return false;
}

std::vector<std::pair<GameAction, GameAction>> InputManager::GetAllConflicts() const {
    std::vector<std::pair<GameAction, GameAction>> conflicts;
    for (const auto& [k1, v1] : reverse_bindings_) {
        if (v1.size() > 1) {
            for (size_t i = 0; i < v1.size(); ++i) {
                for (size_t j = i + 1; j < v1.size(); ++j) {
                    conflicts.push_back({v1[i], v1[j]});
                }
            }
        }
    }
    return conflicts;
}

void InputManager::BeginFrame() {
    for (int i = 0; i < static_cast<int>(GameAction::kCount); ++i) {
        GameAction action = static_cast<GameAction>(i);
        action_just_pressed_[action] = false;
        action_just_released_[action] = false;
    }
}

void InputManager::ProcessEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        if (event.key.repeat) return;
        SDL_Scancode scancode = event.key.keysym.scancode;
        bool pressed = (event.type == SDL_KEYDOWN);
        
        auto it = reverse_bindings_.find(scancode);
        if (it != reverse_bindings_.end()) {
            for (GameAction action : it->second) {
                if (pressed) {
                    if (!action_held_[action]) {
                        action_held_[action] = true;
                        action_just_pressed_[action] = true;
                    }
                } else {
                    if (action_held_[action]) {
                        action_held_[action] = false;
                        action_just_released_[action] = true;
                    }
                }
            }
        }
    } else if (event.type == SDL_CONTROLLERDEVICEADDED) {
        if (!controller_) {
            controller_ = SDL_GameControllerOpen(event.cdevice.which);
        }
    } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
        if (controller_ && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller_))) {
            SDL_GameControllerClose(controller_);
            controller_ = nullptr;
        }
    } else if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) {
        ProcessGamepadButton(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN);
    } else if (event.type == SDL_CONTROLLERAXISMOTION) {
        ProcessGamepadAxis(event.caxis.axis, event.caxis.value);
    }
}

void InputManager::ProcessGamepadButton(int button, bool pressed) {
    auto it = gamepad_profile_.button_map.find(button);
    if (it != gamepad_profile_.button_map.end()) {
        GameAction action = it->second;
        if (pressed) {
            if (!action_held_[action]) {
                action_held_[action] = true;
                action_just_pressed_[action] = true;
            }
        } else {
            if (action_held_[action]) {
                action_held_[action] = false;
                action_just_released_[action] = true;
            }
        }
    }
}

void InputManager::ProcessGamepadAxis(int axis, int16_t value) {
    float norm_value = static_cast<float>(value) / 32767.0f;
    bool is_pressed = std::abs(norm_value) > gamepad_profile_.deadzone;
    
    if (axis == gamepad_profile_.dpad_horizontal_axis) {
        bool left_pressed = is_pressed && norm_value < 0;
        bool right_pressed = is_pressed && norm_value > 0;
        
        GameAction l_action = GameAction::kDpadLeft;
        GameAction r_action = GameAction::kDpadRight;
        
        if (left_pressed && !action_held_[l_action]) { action_held_[l_action] = true; action_just_pressed_[l_action] = true; }
        else if (!left_pressed && action_held_[l_action]) { action_held_[l_action] = false; action_just_released_[l_action] = true; }
        
        if (right_pressed && !action_held_[r_action]) { action_held_[r_action] = true; action_just_pressed_[r_action] = true; }
        else if (!right_pressed && action_held_[r_action]) { action_held_[r_action] = false; action_just_released_[r_action] = true; }
    } else if (axis == gamepad_profile_.dpad_vertical_axis) {
        bool up_pressed = is_pressed && norm_value < 0;
        bool down_pressed = is_pressed && norm_value > 0;
        
        GameAction u_action = GameAction::kDpadUp;
        GameAction d_action = GameAction::kDpadDown;
        
        if (up_pressed && !action_held_[u_action]) { action_held_[u_action] = true; action_just_pressed_[u_action] = true; }
        else if (!up_pressed && action_held_[u_action]) { action_held_[u_action] = false; action_just_released_[u_action] = true; }
        
        if (down_pressed && !action_held_[d_action]) { action_held_[d_action] = true; action_just_pressed_[d_action] = true; }
        else if (!down_pressed && action_held_[d_action]) { action_held_[d_action] = false; action_just_released_[d_action] = true; }
    }
}

bool InputManager::IsActionPressed(GameAction action) const {
    auto it = action_held_.find(action);
    return it != action_held_.end() && it->second;
}

bool InputManager::IsActionJustPressed(GameAction action) const {
    auto it = action_just_pressed_.find(action);
    return it != action_just_pressed_.end() && it->second;
}

bool InputManager::IsActionJustReleased(GameAction action) const {
    auto it = action_just_released_.find(action);
    return it != action_just_released_.end() && it->second;
}

unboundmp::emulator::InputState InputManager::GetEmulatorInputState() const {
    unboundmp::emulator::InputState state;
    if (IsActionPressed(GameAction::kButtonA)) state.Press(unboundmp::emulator::GbaButton::kA);
    if (IsActionPressed(GameAction::kButtonB)) state.Press(unboundmp::emulator::GbaButton::kB);
    if (IsActionPressed(GameAction::kButtonL)) state.Press(unboundmp::emulator::GbaButton::kL);
    if (IsActionPressed(GameAction::kButtonR)) state.Press(unboundmp::emulator::GbaButton::kR);
    if (IsActionPressed(GameAction::kButtonStart)) state.Press(unboundmp::emulator::GbaButton::kStart);
    if (IsActionPressed(GameAction::kButtonSelect)) state.Press(unboundmp::emulator::GbaButton::kSelect);
    if (IsActionPressed(GameAction::kDpadUp)) state.Press(unboundmp::emulator::GbaButton::kUp);
    if (IsActionPressed(GameAction::kDpadDown)) state.Press(unboundmp::emulator::GbaButton::kDown);
    if (IsActionPressed(GameAction::kDpadLeft)) state.Press(unboundmp::emulator::GbaButton::kLeft);
    if (IsActionPressed(GameAction::kDpadRight)) state.Press(unboundmp::emulator::GbaButton::kRight);
    return state;
}

void InputManager::SetGamepadProfile(const GamepadProfile& profile) {
    gamepad_profile_ = profile;
}

GamepadProfile InputManager::GetGamepadProfile() const {
    return gamepad_profile_;
}

bool InputManager::IsGamepadConnected() const {
    return controller_ != nullptr;
}

std::string InputManager::GetGamepadName() const {
    if (controller_) {
        const char* name = SDL_GameControllerName(controller_);
        return name ? name : "Unknown Gamepad";
    }
    return "No Gamepad Connected";
}

} // namespace unboundmp::input
