def update_char_select():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    new_content = '''#include "ui/screens/character_select_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/screens/character_creation_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"

namespace unboundmp::ui {

CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine) : UIScreen("CharacterSelectScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex, true);
    
    auto panel = std::make_shared<Panel>();
    panel->SetBounds({0, 0, 800, 600});
    panel->SetAnchor(AnchorPoint::Center);
    panel->SetBgColor({0,0,0,0}); // transparent panel so background shows
    
    auto title = std::make_shared<Label>();
    title->SetText("Select Character");
    title->SetBounds({0, 50, 800, 40});
    title->SetAlignment(Alignment::Center);
    panel->AddChild(title);
    
    auto chars = std::make_shared<HorizontalLayout>();
    chars->SetBounds({100, 150, 600, 300});
    chars->SetSpacing(20);
    
    for(int i=0; i<2; i++) {
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

    // Create New Character Card
    auto new_card = std::make_shared<Panel>();
    new_card->SetBounds({0, 0, 180, 250});
    new_card->SetBorderThickness(2);
    new_card->SetCornerRadius(8);
    
    auto new_name = std::make_shared<Label>();
    new_name->SetText("Empty Slot");
    new_name->SetBounds({0, 200, 180, 30});
    new_name->SetAlignment(Alignment::Center);
    
    auto create_btn = std::make_shared<Button>();
    create_btn->SetText("Create");
    create_btn->SetBounds({40, 100, 100, 40});
    create_btn->OnClick([this]() {
        engine_->GetScreens().Push(std::make_unique<CharacterCreationScreen>(engine_));
    });
    
    new_card->AddChild(new_name);
    new_card->AddChild(create_btn);
    chars->AddChild(new_card);
    
    panel->AddChild(chars);
    anchor->AddChild(bg_img);
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
    with open(path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    print('Updated CharacterSelectScreen')

update_char_select()
