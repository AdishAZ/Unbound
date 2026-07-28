content = '''#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"

namespace unboundmp::ui {

class UIEngine;

class CreateAccountScreen : public UIScreen {
 public:
    explicit CreateAccountScreen(UIEngine* engine);

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

 private:
    UIEngine* engine_;
    std::shared_ptr<AnchorLayout> root_layout_;
    
    std::shared_ptr<TextBox> username_input_;
    std::shared_ptr<TextBox> password_input_;
    std::shared_ptr<TextBox> confirm_password_input_;
    std::shared_ptr<Label> status_label_;
};

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/include/ui/screens/create_account_screen.h', 'w') as f:
    f.write(content)
