content_cpp = '''#include "ui/screens/character_creation_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"
#include <iostream>

namespace unboundmp::ui {

CharacterCreationScreen::CharacterCreationScreen(UIEngine* engine) : UIScreen("CharacterCreationScreen"), engine_(engine) {
    auto anchor = std::make_shared<AnchorLayout>();
    anchor->SetWidthPolicy(SizePolicy::Expand);
    anchor->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex);
    
    auto panel = std::make_shared<Panel>();
    panel->SetBounds({0, 0, 400, 300});
    panel->SetAnchor(AnchorPoint::Center);
    
    auto title = std::make_shared<Label>();
    title->SetText("Create Character");
    title->SetBounds({0, 20, 400, 40});
    title->SetAlignment(Alignment::Center);
    
    auto name_lbl = std::make_shared<Label>();
    name_lbl->SetText("Character Name");
    name_lbl->SetBounds({50, 100, 300, 20});
    
    name_input_ = std::make_shared<TextBox>();
    name_input_->SetBounds({50, 130, 300, 40});
    name_input_->SetPlaceholder("Enter name...");
    
    status_label_ = std::make_shared<Label>();
    status_label_->SetBounds({50, 180, 300, 20});
    status_label_->SetColor(Color::DarkError);
    
    auto btn_layout = std::make_shared<HorizontalLayout>();
    btn_layout->SetBounds({50, 220, 300, 40});
    btn_layout->SetSpacing(20);
    
    auto create_btn = std::make_shared<Button>();
    create_btn->SetText("Start Journey");
    create_btn->SetBounds({0, 0, 140, 40});
    create_btn->OnClick([this]() {
        auto client = engine_->GetNetworkClient();
        if (client) {
            network::CreateCharacterRequestPacket req;
            req.name = name_input_->GetText();
            req.appearance = "default";
            
            network::Packet p;
            p.type = network::PacketType::kCreateCharacterRequest;
            p.payload = req.Serialize();
            client->SendPacket(p);
            
            status_label_->SetText("Creating...");
            status_label_->SetColor(Color::DarkText);
        }
    });
    
    auto back_btn = std::make_shared<Button>();
    back_btn->SetText("Cancel");
    back_btn->SetBounds({0, 0, 140, 40});
    back_btn->OnClick([this]() {
        engine_->GetScreens().Pop();
    });
    
    btn_layout->AddChild(back_btn);
    btn_layout->AddChild(create_btn);
    
    panel->AddChild(title);
    panel->AddChild(name_lbl);
    panel->AddChild(name_input_);
    panel->AddChild(status_label_);
    panel->AddChild(btn_layout);
    
    anchor->AddChild(bg_img);
    anchor->AddChild(panel);
    root_layout_ = anchor;
}

void CharacterCreationScreen::OnEnter() {}
void CharacterCreationScreen::OnExit() {}
void CharacterCreationScreen::OnPause() {}
void CharacterCreationScreen::OnResume() {}

void CharacterCreationScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void CharacterCreationScreen::Render(const RenderContext& ctx) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool CharacterCreationScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) return root_layout_->HandleInput(event);
    return false;
}

void CharacterCreationScreen::Update(float dt) {
    if (root_layout_) root_layout_->Update(dt);
    
    auto client = engine_->GetNetworkClient();
    if (client) {
        while (auto opt_packet = client->ReceivePacket()) {
            if (opt_packet->type == network::PacketType::kCreateCharacterResponse) {
                auto resp = network::CreateCharacterResponsePacket::Deserialize(opt_packet->payload);
                if (resp.success) {
                    // Instantly select the new character
                    network::SelectCharacterRequestPacket req;
                    req.character_id = resp.character_id;
                    network::Packet p;
                    p.type = network::PacketType::kSelectCharacterRequest;
                    p.payload = req.Serialize();
                    client->SendPacket(p);
                } else {
                    status_label_->SetText(resp.message);
                    status_label_->SetColor(Color::DarkError);
                }
            } else if (opt_packet->type == network::PacketType::kSelectCharacterResponse) {
                auto resp = network::SelectCharacterResponsePacket::Deserialize(opt_packet->payload);
                if (resp.success) {
                    // New character boot sequence!
                    engine_->GetScreens().Replace(std::make_unique<GameScreen>(engine_, false)); // false = don't load state, show title screen
                } else {
                    status_label_->SetText(resp.message);
                    status_label_->SetColor(Color::DarkError);
                }
            }
        }
    }
}

}
'''
with open('d:/Unbound/pokemon/cpp/src/ui/screens/character_creation_screen.cpp', 'w') as f:
    f.write(content_cpp)

content_h = '''#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"

namespace unboundmp::ui {

class UIEngine;

class CharacterCreationScreen : public UIScreen {
 public:
    explicit CharacterCreationScreen(UIEngine* engine);

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
    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<TextBox> name_input_;
    std::shared_ptr<Label> status_label_;
};

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/include/ui/screens/character_creation_screen.h', 'w') as f:
    f.write(content_h)

