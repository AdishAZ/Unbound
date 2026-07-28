#include "ui/screens/settings_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "core/event_system.h"

namespace unboundmp::ui {

SettingsScreen::SettingsScreen(UIEngine* engine) : UIScreen("SettingsScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);

    auto panel_ = std::make_shared<Window>("settings_win");
    panel_->SetTitle(L("settings.title"));
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 600, 450});
    panel_->OnClose([this]() {
        engine_->GetScreens().Pop();
    });

    auto vert = std::make_shared<VerticalLayout>();
    vert->SetBounds({10, 40, 580, 400}); // below title bar

    auto tabs = std::make_shared<TabControl>("settings_tabs");
    tabs->SetBounds({0, 0, 580, 350});

    // We build the contents inline or using helper methods
    auto video_vert = std::make_shared<VerticalLayout>();
    video_vert->SetWidthPolicy(SizePolicy::Expand);
    video_vert->SetHeightPolicy(SizePolicy::Expand);
    video_vert->SetPadding(Padding::All(10));
    video_vert->SetSpacing(10);
    
    auto fs = std::make_shared<Checkbox>(); fs->SetLabel(L("settings.fullscreen")); fs->SetBounds({0, 0, 200, 20});
    auto vs = std::make_shared<Checkbox>(); vs->SetLabel(L("settings.vsync")); vs->SetBounds({0, 0, 200, 20});
    video_vert->AddChild(fs); video_vert->AddChild(vs);
    tabs->AddTab("Video", video_vert);

    auto ui_vert = std::make_shared<VerticalLayout>();
    ui_vert->SetWidthPolicy(SizePolicy::Expand);
    ui_vert->SetHeightPolicy(SizePolicy::Expand);
    ui_vert->SetPadding(Padding::All(10));
    
    auto theme_combo = std::make_shared<ComboBox>("theme_select");
    theme_combo->SetBounds({0, 0, 200, 30});
    theme_combo->AddItem("Dark Theme");
    theme_combo->AddItem("Light Theme");
    theme_combo->SetSelectedIndex(0);
    ui_vert->AddChild(theme_combo);
    tabs->AddTab("UI", ui_vert);

    auto save_vert = std::make_shared<VerticalLayout>();
    save_vert->SetWidthPolicy(SizePolicy::Expand);
    save_vert->SetHeightPolicy(SizePolicy::Expand);
    save_vert->SetPadding(Padding::All(10));
    save_vert->SetSpacing(10);
    
    auto return_btn = std::make_shared<Button>();
    return_btn->SetText("Save and Return to Login");
    return_btn->SetBounds({0, 0, 250, 40});
    return_btn->OnClick([this]() {
        core::EventSystem::GetInstance().Publish(core::EventType::kReturnToLoginRequested, core::EmptyEvent());
    });
    
    auto exit_btn = std::make_shared<Button>();
    exit_btn->SetText("Save and Exit");
    exit_btn->SetBounds({0, 0, 250, 40});
    exit_btn->OnClick([this]() {
        core::EventSystem::GetInstance().Publish(core::EventType::kSaveAndExitRequested, core::EmptyEvent());
    });
    
    save_vert->AddChild(return_btn);
    save_vert->AddChild(exit_btn);
    tabs->AddTab("Save/Exit", save_vert);

    auto btn_layout = std::make_shared<HorizontalLayout>();
    btn_layout->SetBounds({0, 0, 580, 40});
    
    auto close_btn = std::make_shared<Button>();
    close_btn->SetText("Close");
    close_btn->SetBounds({480, 10, 80, 30});
    close_btn->OnClick([this]() {
        engine_->GetScreens().Pop();
    });
    btn_layout->AddChild(close_btn);

    vert->AddChild(tabs);
    vert->AddChild(btn_layout);
    panel_->AddChild(vert);
    anchor->AddChild(panel_);
    
    root_layout_ = anchor;
}

void SettingsScreen::OnEnter() {}
void SettingsScreen::OnExit() {}
void SettingsScreen::OnPause() {}
void SettingsScreen::OnResume() {}

void SettingsScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void SettingsScreen::Render(const RenderContext& ctx) {
    // Dark overlay for background
    ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {0, 0, 0, 150});
    
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool SettingsScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) return root_layout_->HandleInput(event);
    return false;
}

void SettingsScreen::Update(float dt) {
    if (root_layout_) root_layout_->Update(dt);
}

void SettingsScreen::BuildVideoTab() {}
void SettingsScreen::BuildAudioTab() {}
void SettingsScreen::BuildControlsTab() {}
void SettingsScreen::BuildNetworkTab() {}
void SettingsScreen::BuildUITab() {}
void SettingsScreen::BuildDeveloperTab() {}
void SettingsScreen::BuildGameplayTab() {}
void SettingsScreen::SwitchTab(size_t index) {}

} // namespace unboundmp::ui
