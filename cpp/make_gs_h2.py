content = '''#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"
#include "emulator/input_state.h"

namespace unboundmp::ui {

class UIEngine;
class WindowManager;

class GameScreen : public UIScreen {
 public:
    explicit GameScreen(UIEngine* engine, bool load_save_state = true);

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;
    
    bool ShouldLoadSaveState() const { return load_save_state_; }
    unboundmp::emulator::InputState GetVirtualInput() const { return virtual_input_; }

 private:
    void BuildHUD();

    UIEngine* engine_;
    bool load_save_state_;
    unboundmp::emulator::InputState virtual_input_;
    std::shared_ptr<Container> hud_overlay_;
    std::unique_ptr<WindowManager> window_manager_;
    
    std::vector<std::pair<std::shared_ptr<Button>, unboundmp::emulator::GbaButton>> virtual_buttons_;
};

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/include/ui/screens/game_screen.h', 'w') as f:
    f.write(content)
