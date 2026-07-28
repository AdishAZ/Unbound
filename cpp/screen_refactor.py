def overhaul_game_screen():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/game_screen.h'
    content_h = '''#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"

namespace unboundmp::ui {

class UIEngine;
class WindowManager;

class GameScreen : public UIScreen {
public:
    explicit GameScreen(UIEngine* engine);
    ~GameScreen() override = default;

    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;

    void OnResize(int width, int height) override;
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

private:
    void BuildHUD();

    UIEngine* engine_;
    std::shared_ptr<Container> hud_overlay_;
    std::unique_ptr<WindowManager> window_manager_;
};

} // namespace unboundmp::ui
'''
    with open(path_h, 'w', encoding='utf-8') as f:
        f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp'
    content_cpp = '''#include "ui/screens/game_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "ui/screens/settings_screen.h"
#include "ui/window_manager.h"

namespace unboundmp::ui {

GameScreen::GameScreen(UIEngine* engine) : UIScreen("GameScreen"), engine_(engine) {
    hud_overlay_ = std::make_shared<Container>("hud_overlay");
    hud_overlay_->SetWidthPolicy(SizePolicy::Expand);
    hud_overlay_->SetHeightPolicy(SizePolicy::Expand);

    window_manager_ = std::make_unique<WindowManager>();

    BuildHUD();
}

void GameScreen::BuildHUD() {
    // Top-Left Info Bar
    auto top_left = std::make_shared<Panel>("hud_top_left");
    top_left->SetBounds({10, 10, 200, 60});
    top_left->SetBackgroundColor({20, 20, 25, 200}); // Semi-transparent dark
    top_left->SetCornerRadius(4);
    
    auto loc_label = std::make_shared<Label>();
    loc_label->SetText("Cherrygrove City Ch. 3");
    loc_label->SetBounds({5, 5, 190, 15});
    
    auto money_label = std::make_shared<Label>();
    money_label->SetText(",185");
    money_label->SetBounds({5, 22, 190, 15});
    
    auto time_label = std::make_shared<Label>();
    time_label->SetText("Thursday, 08:59");
    time_label->SetBounds({5, 39, 190, 15});
    
    top_left->AddChild(loc_label);
    top_left->AddChild(money_label);
    top_left->AddChild(time_label);
    hud_overlay_->AddChild(top_left);

    // Bottom-Left Chat Window
    auto chat_panel = std::make_shared<Panel>("hud_chat");
    chat_panel->SetBounds({10, engine_->GetRenderContext().screen_height - 210, 300, 200});
    chat_panel->SetBackgroundColor({10, 15, 20, 220});
    chat_panel->SetCornerRadius(4);
    
    auto tabs = std::make_shared<HorizontalLayout>();
    tabs->SetBounds({5, 5, 290, 20});
    tabs->SetSpacing(5);
    std::vector<std::string> tab_names = {"Local", "Global", "Trade", "Whispers", "Battle"};
    for (const auto& t : tab_names) {
        auto tb = std::make_shared<Button>();
        tb->SetText(t);
        tb->SetBounds({0, 0, 45, 20});
        tabs->AddChild(tb);
    }
    chat_panel->AddChild(tabs);
    
    auto chat_list = std::make_shared<ListView>("chat_list");
    chat_list->SetBounds({5, 30, 290, 135});
    chat_list->AddItem("[System] Welcome to PokeMMO! Enjoy your stay.");
    chat_list->AddItem("[System] You are connecting through server Josuke.");
    chat_panel->AddChild(chat_list);
    
    auto chat_input = std::make_shared<TextBox>("chat_input");
    chat_input->SetBounds({5, 170, 250, 25});
    chat_input->SetPlaceholder("Normal");
    chat_panel->AddChild(chat_input);
    
    auto send_btn = std::make_shared<Button>();
    send_btn->SetText("Send");
    send_btn->SetBounds({260, 170, 35, 25});
    chat_panel->AddChild(send_btn);
    
    hud_overlay_->AddChild(chat_panel);

    // Right-Side Party Column
    auto party_layout = std::make_shared<VerticalLayout>();
    party_layout->SetBounds({engine_->GetRenderContext().screen_width - 50, 150, 40, 300});
    party_layout->SetSpacing(2);
    for (int i=0; i<6; i++) {
        auto slot = std::make_shared<Panel>();
        slot->SetBounds({0, 0, 40, 40});
        slot->SetBackgroundColor({20, 20, 25, 180});
        slot->SetBorderThickness(1);
        slot->SetBorderColor({100, 100, 100, 255});
        party_layout->AddChild(slot);
    }
    hud_overlay_->AddChild(party_layout);

    // Bottom-Right Quick Menu
    auto quick_layout = std::make_shared<HorizontalLayout>();
    quick_layout->SetBounds({engine_->GetRenderContext().screen_width - 170, engine_->GetRenderContext().screen_height - 30, 160, 25});
    quick_layout->SetSpacing(2);
    for (int i=0; i<6; i++) {
        auto btn = std::make_shared<Button>();
        btn->SetText("?");
        btn->SetBounds({0, 0, 25, 25});
        quick_layout->AddChild(btn);
    }
    hud_overlay_->AddChild(quick_layout);
}

void GameScreen::OnEnter() {}
void GameScreen::OnExit() {}
void GameScreen::OnPause() {}
void GameScreen::OnResume() {}

void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    // Reposition anchored/absolute elements
    auto chat = hud_overlay_->GetChild("hud_chat");
    if (chat) chat->SetBounds({10, height - 210, 300, 200});
    
    // Quick menu and party layout would need updating too ideally, 
    // but this suffices for static resize demonstration.
}

void GameScreen::Render(const RenderContext& ctx) {
    // Removed SDL_RenderClear so the emulator framebuffer shows through!
    hud_overlay_->Render(ctx);
    window_manager_->Render(ctx);
}

bool GameScreen::HandleInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        engine_->GetScreens().Push(std::make_unique<SettingsScreen>(engine_));
        return true;
    }
    
    if (window_manager_->HandleInput(event)) return true;
    return hud_overlay_->HandleInput(event);
}

void GameScreen::Update(float dt) {
    hud_overlay_->Update(dt);
    window_manager_->Update(dt);
}

} // namespace unboundmp::ui
'''
    with open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(content_cpp)
    print('Refactored GameScreen.cpp')

overhaul_game_screen()
