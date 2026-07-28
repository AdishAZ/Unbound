#pragma once

#include <SDL.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "emulator/emulator_core.h"

namespace unboundmp::input {

enum class GameAction {
    // GBA buttons
    kButtonA, kButtonB, kButtonL, kButtonR,
    kButtonStart, kButtonSelect,
    kDpadUp, kDpadDown, kDpadLeft, kDpadRight,
    // Hotkeys
    kSaveState, kLoadState,
    kFastForward, kToggleHUD, kToggleDevOverlay,
    kMute, kScreenshot,
    kOpenSettings, kOpenChat,
    kCount
};

struct GamepadProfile {
    std::string name;
    float deadzone = 0.25f;
    // SDL_GameControllerButton -> GameAction
    std::unordered_map<int, GameAction> button_map;
    // Axis mapping: which SDL axis controls dpad
    int dpad_horizontal_axis = 0; // SDL_CONTROLLER_AXIS_LEFTX
    int dpad_vertical_axis = 1;   // SDL_CONTROLLER_AXIS_LEFTY
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    void Initialize();
    void Shutdown();
    
    // Key binding
    void SetKeyBinding(GameAction action, SDL_Scancode key);
    SDL_Scancode GetKeyBinding(GameAction action) const;
    std::string GetKeyName(GameAction action) const;
    std::string GetActionName(GameAction action) const;
    void SetDefaultBindings();
    
    // Conflict detection
    bool HasConflict(GameAction action, SDL_Scancode key) const;
    std::vector<std::pair<GameAction, GameAction>> GetAllConflicts() const;
    
    // Input processing
    void ProcessEvent(const SDL_Event& event);
    bool IsActionPressed(GameAction action) const;
    bool IsActionJustPressed(GameAction action) const; // true only on the frame it was pressed
    bool IsActionJustReleased(GameAction action) const;
    
    // Get emulator input state from current action state
    unboundmp::emulator::InputState GetEmulatorInputState() const;
    
    // Gamepad
    void SetGamepadProfile(const GamepadProfile& profile);
    GamepadProfile GetGamepadProfile() const;
    bool IsGamepadConnected() const;
    std::string GetGamepadName() const;
    
    // Frame management
    void BeginFrame(); // clear just_pressed/just_released
    
private:
    // Key bindings
    std::unordered_map<GameAction, SDL_Scancode> key_bindings_;
    
    // State tracking
    std::unordered_map<GameAction, bool> action_held_;
    std::unordered_map<GameAction, bool> action_just_pressed_;
    std::unordered_map<GameAction, bool> action_just_released_;
    
    // Reverse lookup: scancode -> actions
    std::unordered_map<SDL_Scancode, std::vector<GameAction>> reverse_bindings_;
    void RebuildReverseLookup();
    
    // Gamepad
    GamepadProfile gamepad_profile_;
    SDL_GameController* controller_ = nullptr;
    void ProcessGamepadButton(int button, bool pressed);
    void ProcessGamepadAxis(int axis, int16_t value);
};

} // namespace unboundmp::input
