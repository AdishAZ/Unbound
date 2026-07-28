def fix_char_select2():
    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp'
    full_new = '''#include "ui/screens/character_select_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"

namespace unboundmp::ui {

CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine) : UIScreen("CharacterSelectScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);
    
    auto panel = std::make_shared<Panel>();
    panel->SetBounds({0, 0, 800, 600});
    
    auto title = std::make_shared<Label>();
    title->SetText("Select Character");
    title->SetBounds({0, 50, 800, 40});
    title->SetAlignment(Alignment::Center);
    panel->AddChild(title);
    
    auto chars = std::make_shared<HorizontalLayout>();
    chars->SetBounds({100, 150, 600, 300});
    chars->SetSpacing(20);
    
    for(int i=0; i<3; i++) {
        auto card = std::make_shared<Panel>();
        card->SetBounds({0, 0, 180, 250});
        card->SetBorderThickness(2);
        card->SetCornerRadius(8);
        
        auto name = std::make_shared<Label>();
        name->SetText("Player " + std::to_string(i+1));
        name->SetBounds({0, 200, 180, 30});
        name->SetAlignment(Alignment::Center);
        
        auto btn = std::make_shared<Button>();
        btn->SetText("Play");
        btn->SetBounds({40, 100, 100, 40});
        btn->OnClick([this]() {
            engine_->GetScreens().Push(std::make_unique<GameScreen>(engine_));
        });
        
        card->AddChild(name);
        card->AddChild(btn);
        chars->AddChild(card);
    }
    
    panel->AddChild(chars);
    anchor->AddChild(panel);
    root_layout_ = anchor;
}

void CharacterSelectScreen::OnEnter() {}
void CharacterSelectScreen::OnExit() {}
void CharacterSelectScreen::OnPause() {}
void CharacterSelectScreen::OnResume() {}

void CharacterSelectScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void CharacterSelectScreen::Render(const RenderContext& ctx) {
    ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {30, 30, 40, 255});
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool CharacterSelectScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) return root_layout_->HandleInput(event);
    return false;
}

void CharacterSelectScreen::Update(float dt) {
    if (root_layout_) root_layout_->Update(dt);
}

} // namespace unboundmp::ui
'''

    with open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(full_new)
    print('Fixed character select screen cleanly')

fix_char_select2()
