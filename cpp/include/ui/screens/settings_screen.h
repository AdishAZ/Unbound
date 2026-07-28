#pragma once
#include "ui/screen.h"
#include "ui/widgets.h"
#include "ui/ui_types.h"
#include <memory>
#include <vector>

namespace unboundmp::ui {
class UIEngine;

class SettingsScreen : public UIScreen {
public:
    explicit SettingsScreen(UIEngine* engine);
    ~SettingsScreen() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    void SwitchTab(size_t index);
    
    void BuildVideoTab();
    void BuildAudioTab();
    void BuildControlsTab();
    void BuildNetworkTab();
    void BuildUITab();
    void BuildDeveloperTab();
    void BuildGameplayTab();

    UIEngine* engine_;
    
    std::shared_ptr<Container> root_layout_;
    std::shared_ptr<Label> title_label_;
    std::shared_ptr<Container> tab_bar_;
    std::vector<std::shared_ptr<Button>> tab_buttons_;
    
    std::shared_ptr<Panel> content_panel_;
    std::vector<std::shared_ptr<Container>> tab_contents_;

    std::shared_ptr<Container> button_row_;
    std::shared_ptr<Button> apply_button_;
    std::shared_ptr<Button> reset_button_;
    std::shared_ptr<Button> defaults_button_;
};
} // namespace unboundmp::ui
