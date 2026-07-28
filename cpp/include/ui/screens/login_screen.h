#pragma once
#include "ui/screen.h"
#include "ui/widgets.h"
#include "ui/ui_types.h"
#include <memory>
#include <string>

namespace unboundmp::ui {

class UIEngine;

enum class LoginState {
    Idle,
    Connecting,
    Authenticating,
    LoadingCharacter,
    JoiningWorld,
    Connected,
    Error
};

class LoginScreen : public UIScreen {
public:
    explicit LoginScreen(UIEngine* engine);
    ~LoginScreen() override = default;

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
    LoginState state_;
    float state_timer_;

    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<Panel> panel_;
    std::shared_ptr<Label> title_label_;
    std::shared_ptr<Label> user_label_;
    std::shared_ptr<TextBox> username_input_;
    std::shared_ptr<Label> pass_label_;
    std::shared_ptr<TextBox> password_input_;
    std::shared_ptr<Checkbox> remember_checkbox_;
    std::shared_ptr<Button> login_button_;
    std::shared_ptr<Label> status_label_;
    std::shared_ptr<ProgressBar> connect_progress_;
    
    std::vector<uint64_t> subscriptions_;
};

} // namespace unboundmp::ui
