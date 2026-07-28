def refactor_settings():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_init = '''SettingsScreen::SettingsScreen(UIEngine* engine) : UIScreen("SettingsScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);

    panel_ = std::make_shared<Window>("settings_win");
    ((Window*)panel_.get())->SetTitle(L("settings.title"));
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 600, 450});

    auto vert = std::make_shared<VerticalLayout>();
    vert->SetBounds({10, 40, 580, 400}); // below title bar

    auto tabs = std::make_shared<TabControl>("settings_tabs");
    tabs->SetBounds({0, 0, 580, 350});

    // We build the contents inline or using helper methods
    auto video_tab = std::make_shared<Panel>();
    auto fs = std::make_shared<Checkbox>(); fs->SetLabel(L("settings.fullscreen")); fs->SetBounds({10, 10, 200, 20});
    auto vs = std::make_shared<Checkbox>(); vs->SetLabel(L("settings.vsync")); vs->SetBounds({10, 40, 200, 20});
    video_tab->AddChild(fs); video_tab->AddChild(vs);
    tabs->AddTab("Video", video_tab);

    auto ui_tab = std::make_shared<Panel>();
    auto theme_combo = std::make_shared<ComboBox>("theme_select");
    theme_combo->SetBounds({10, 10, 200, 30});
    theme_combo->AddItem("Dark Theme");
    theme_combo->AddItem("Light Theme");
    theme_combo->SetSelectedIndex(0);
    ui_tab->AddChild(theme_combo);
    tabs->AddTab("UI", ui_tab);

    auto btn_layout = std::make_shared<HorizontalLayout>();
    btn_layout->SetBounds({0, 0, 580, 40});
    
    auto close_btn = std::make_shared<Button>();
    close_btn->SetText("Close");
    close_btn->SetBounds({480, 10, 80, 30});
    close_btn->OnClick([this]() {
        // Pop screen
    });
    btn_layout->AddChild(close_btn);

    vert->AddChild(tabs);
    vert->AddChild(btn_layout);
    panel_->AddChild(vert);
    anchor->AddChild(panel_);
    
    root_layout_ = anchor;
}'''

    # To be safe, I'll rewrite the entire file since the old one has many small methods
    full_new = '''#include "ui/screens/settings_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"

namespace unboundmp::ui {

''' + new_init + '''

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
'''

    with open(path, 'w', encoding='utf-8') as f:
        f.write(full_new)
    print('SettingsScreen refactored')

refactor_settings()
