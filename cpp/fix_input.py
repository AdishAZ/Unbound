def fix_input_manager():
    path = 'd:/Unbound/pokemon/cpp/src/input/input_manager.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    old_func = '''unboundmp::emulator::InputState InputManager::GetEmulatorInputState() const {
    unboundmp::emulator::InputState state;
    state.a = IsActionPressed(GameAction::kButtonA);
    state.b = IsActionPressed(GameAction::kButtonB);
    state.l = IsActionPressed(GameAction::kButtonL);
    state.r = IsActionPressed(GameAction::kButtonR);
    state.start = IsActionPressed(GameAction::kButtonStart);
    state.select = IsActionPressed(GameAction::kButtonSelect);
    state.up = IsActionPressed(GameAction::kDpadUp);
    state.down = IsActionPressed(GameAction::kDpadDown);
    state.left = IsActionPressed(GameAction::kDpadLeft);
    state.right = IsActionPressed(GameAction::kDpadRight);
    return state;
}'''

    new_func = '''unboundmp::emulator::InputState InputManager::GetEmulatorInputState() const {
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
}'''

    if old_func in content:
        content = content.replace(old_func, new_func)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print("Fixed input_manager.cpp")

fix_input_manager()
