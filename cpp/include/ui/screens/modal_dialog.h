#pragma once
#include "ui/screen.h"
#include "ui/widgets.h"
#include <string>

namespace unboundmp::ui {
class UIEngine;
class ModalDialog : public UIScreen {
public:
    ModalDialog(UIEngine* engine, const std::string& title, const std::string& message);
    void OnEnter() override;
    void OnExit() override;
    void OnPause() override {}
    void OnResume() override {}
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;
    
    void AddButton(const std::string& text, ClickCallback on_click);
    
private:
    UIEngine* engine_;
    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<Panel> panel_;
    std::shared_ptr<HorizontalLayout> button_layout_;
};
} // namespace unboundmp::ui
