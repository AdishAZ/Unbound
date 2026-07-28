content = '''#include "ui/screens/game_screen.h"
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
    
    auto tabs = std::make_shared<HorizontalLayout>("chat_tabs");
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
    
    auto send_btn = std::make_shared<Button>("send_btn");
    send_btn->SetText("Send");
    send_btn->SetBounds({260, 170, 35, 25});
    chat_panel->AddChild(send_btn);
    
    hud_overlay_->AddChild(chat_panel);

    // Right-Side Party Column
    auto party_layout = std::make_shared<VerticalLayout>("party_col");
    party_layout->SetBounds({engine_->GetRenderContext().screen_width - 50, 150, 40, 300});
    party_layout->SetSpacing(2);
    for (int i=0; i<6; i++) {
        auto slot = std::make_shared<Panel>();
        slot->SetBounds({0, 0, 40, 40});
        slot->SetBackgroundColor({20, 20, 25, 180});
        slot->SetBorderThickness(1);
        slot->SetBorderColor({100, 100, 100, 255});
        
        auto p_label = std::make_shared<Label>();
        p_label->SetText("P" + std::to_string(i+1));
        p_label->SetBounds({5, 10, 30, 20});
        slot->AddChild(p_label);
        
        party_layout->AddChild(slot);
    }
    hud_overlay_->AddChild(party_layout);

    // Bottom-Right Quick Menu
    auto quick_layout = std::make_shared<HorizontalLayout>("quick_menu");
    quick_layout->SetBounds({engine_->GetRenderContext().screen_width - 200, engine_->GetRenderContext().screen_height - 30, 190, 25});
    quick_layout->SetSpacing(2);
    
    std::vector<std::string> quick_btns = {"Bag", "Trainer Card", "Friends"};
    for (const auto& name : quick_btns) {
        auto btn = std::make_shared<Button>();
        btn->SetText(name);
        btn->SetBounds({0, 0, 60, 25});
        quick_layout->AddChild(btn);
    }
    hud_overlay_->AddChild(quick_layout);
}

void GameScreen::OnEnter() {
    OnResize(engine_->GetRenderContext().screen_width, engine_->GetRenderContext().screen_height);
}
void GameScreen::OnExit() {}
void GameScreen::OnPause() {}
void GameScreen::OnResume() {}

void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    
    int chat_y = height - 210;
    
    // Reposition anchored/absolute elements
    for (auto& child : hud_overlay_->GetChildren()) {
        if (child->GetId() == "hud_chat") {
            child->SetBounds({10, chat_y, 300, 200});
            for (auto& c : child->GetChildren()) {
                if (c->GetId() == "chat_tabs") c->SetBounds({15, chat_y + 5, 290, 20});
                else if (c->GetId() == "chat_list") c->SetBounds({15, chat_y + 30, 290, 135});
                else if (c->GetId() == "chat_input") c->SetBounds({15, chat_y + 170, 250, 25});
                else if (c->GetId() == "send_btn") c->SetBounds({270, chat_y + 170, 35, 25});
            }
        } else if (child->GetId() == "party_col") {
            child->SetBounds({width - 50, 150, 40, 300});
        } else if (child->GetId() == "quick_menu") {
            child->SetBounds({width - 200, height - 30, 190, 25});
        }
    }
}

void GameScreen::Render(const RenderContext& ctx) {
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
with open('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', 'w') as f:
    f.write(content)
