def update_gamescreen():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/game_screen.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if '#include "ui/window_manager.h"' not in content_h:
        content_h = content_h.replace('#include "ui/screen.h"', '#include "ui/screen.h"\n#include "ui/window_manager.h"')
        content_h = content_h.replace('std::shared_ptr<Container> hud_overlay_;', 'std::shared_ptr<Container> hud_overlay_;\n    std::unique_ptr<WindowManager> window_manager_;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    new_cpp = '''#include "ui/screens/game_screen.h"
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

    // Mock Bottom HUD Bar
    auto bottom_bar = std::make_shared<Panel>("bottom_bar");
    bottom_bar->SetBounds({0, 600 - 60, 800, 60});
    bottom_bar->SetAnchor(AnchorPoint::BottomCenter);
    
    auto hud_layout = std::make_shared<HorizontalLayout>();
    hud_layout->SetBounds({20, 10, 760, 40});
    hud_layout->SetSpacing(10);
    
    auto inv_btn = std::make_shared<Button>("btn_inventory");
    inv_btn->SetText("Inventory (I)");
    inv_btn->SetBounds({0, 0, 120, 30});
    inv_btn->OnClick([this]() {
        // Toggle inventory window
    });
    
    hud_layout->AddChild(inv_btn);
    bottom_bar->AddChild(hud_layout);
    hud_overlay_->AddChild(bottom_bar);

    // Mock Inventory Window
    auto inv_win = std::make_shared<Window>("win_inventory");
    inv_win->SetTitle("Inventory");
    inv_win->SetBounds({50, 50, 300, 400});
    inv_win->SetCloseable(true);
    
    auto inv_layout = std::make_shared<GridLayout>(5, "inv_grid");
    inv_layout->SetBounds({10, 35, 280, 350});
    inv_layout->SetSpacing(5);
    for (int i=0; i<30; i++) {
        auto slot = std::make_shared<Panel>("slot_" + std::to_string(i));
        slot->SetBounds({0,0,40,40});
        slot->SetBorderThickness(1);
        inv_layout->AddChild(slot);
    }
    inv_win->AddChild(inv_layout);
    
    // Mock Chat Window
    auto chat_win = std::make_shared<Window>("win_chat");
    chat_win->SetTitle("Chat");
    chat_win->SetBounds({450, 350, 300, 200});
    chat_win->SetCloseable(false);
    
    auto chat_list = std::make_shared<ListView>("chat_list");
    chat_list->SetBounds({10, 35, 280, 120});
    chat_list->AddItem("[System] Welcome to UnboundMP.");
    chat_list->AddItem("[Global] Player1: LFG route 1");
    chat_win->AddChild(chat_list);
    
    auto chat_input = std::make_shared<TextBox>("chat_input");
    chat_input->SetBounds({10, 160, 280, 30});
    chat_input->SetPlaceholder("Press Enter to chat...");
    chat_win->AddChild(chat_input);

    window_manager_->AddWindow(inv_win);
    window_manager_->AddWindow(chat_win);
}

void GameScreen::OnEnter() {}
void GameScreen::OnExit() {}
void GameScreen::OnPause() {}
void GameScreen::OnResume() {}

void GameScreen::OnResize(int width, int height) {
    hud_overlay_->SetBounds({0, 0, width, height});
    hud_overlay_->InvalidateLayout();
    window_manager_->SetBounds({0, 0, width, height});
}

void GameScreen::Render(const RenderContext& ctx) {
    // Render game world (Mock)
    SDL_SetRenderDrawColor(ctx.renderer, 43, 100, 43, 255); // Grass green
    SDL_RenderClear(ctx.renderer);
    
    hud_overlay_->Render(ctx);
    window_manager_->Render(ctx);
}

bool GameScreen::HandleInput(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        engine_->GetScreens().Push(std::make_unique<SettingsScreen>(engine_));
        return true;
    }
    
    // Handle windows first (Z-Order)
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
        f.write(new_cpp)
    print('GameScreen updated')

update_gamescreen()
