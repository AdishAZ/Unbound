content_h = '''#pragma once

#include "ui/screen.h"
#include "ui/widgets.h"
#include "network/packet.h"

namespace unboundmp::ui {

class UIEngine;

class CharacterSelectScreen : public UIScreen {
 public:
    explicit CharacterSelectScreen(UIEngine* engine);
    
    void OnEnter() override;
    void OnExit() override;
    void OnPause() override;
    void OnResume() override;
    void OnResize(int width, int height) override;
    
    void Render(const RenderContext& ctx) override;
    bool HandleInput(const SDL_Event& event) override;
    void Update(float dt) override;

 private:
    void BuildUI(const std::vector<network::CharacterListResponsePacket::CharacterEntry>& characters);

    UIEngine* engine_;
    std::shared_ptr<AnchorLayout> root_layout_;
    std::shared_ptr<HorizontalLayout> chars_layout_;
    
    enum class State {
        Fetching,
        Loaded
    };
    State state_ = State::Fetching;
};

} // namespace unboundmp::ui
'''
with open('d:/Unbound/pokemon/cpp/include/ui/screens/character_select_screen.h', 'w') as f:
    f.write(content_h)

content_cpp = '''#include "ui/screens/character_select_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/screens/character_creation_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "network/multiplayer_client.h"

namespace unboundmp::ui {

CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine) : UIScreen("CharacterSelectScreen"), engine_(engine) {
    root_layout_ = std::make_shared<AnchorLayout>();
    root_layout_->SetWidthPolicy(SizePolicy::Expand);
    root_layout_->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex);
    
    auto title = std::make_shared<Label>();
    title->SetText(L("char_select.title"));
    title->SetBounds({0, 50, engine->GetRenderContext().screen_width, 40});
    title->SetAlignment(Alignment::Center);
    
    chars_layout_ = std::make_shared<HorizontalLayout>();
    chars_layout_->SetBounds({100, 150, 600, 300});
    chars_layout_->SetSpacing(20);
    
    root_layout_->AddChild(bg_img);
    root_layout_->AddChild(title);
    root_layout_->AddChild(chars_layout_);
}

void CharacterSelectScreen::BuildUI(const std::vector<network::CharacterListResponsePacket::CharacterEntry>& characters) {
    chars_layout_->ClearChildren();
    
    for (const auto& c : characters) {
        auto card = std::make_shared<Panel>();
        card->SetBounds({0, 0, 180, 250});
        card->SetBorderThickness(2);
        card->SetCornerRadius(8);
        
        auto name = std::make_shared<Label>();
        name->SetText(c.name);
        name->SetBounds({0, 170, 180, 30});
        name->SetAlignment(Alignment::Center);
        
        auto btn = std::make_shared<Button>();
        btn->SetText("Play");
        btn->SetBounds({40, 100, 100, 40});
        
        uint64_t char_id = c.id;
        btn->OnClick([this, char_id]() {
            auto client = engine_->GetNetworkClient();
            if (client) {
                network::SelectCharacterRequestPacket req;
                req.character_id = char_id;
                network::Packet p;
                p.type = network::PacketType::kSelectCharacterRequest;
                p.payload = req.Serialize();
                client->SendPacket(p);
            }
        });
        
        card->AddChild(name);
        card->AddChild(btn);
        chars_layout_->AddChild(card);
    }
    
    // Always show a new slot if under some limit (e.g., 3)
    if (characters.size() < 3) {
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
        chars_layout_->AddChild(new_card);
    }
}

void CharacterSelectScreen::OnEnter() {
    auto client = engine_->GetNetworkClient();
    if (client) {
        network::CharacterListRequestPacket req;
        network::Packet p;
        p.type = network::PacketType::kCharacterListRequest;
        p.payload = req.Serialize();
        client->SendPacket(p);
        state_ = State::Fetching;
    }
}

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
    
    auto client = engine_->GetNetworkClient();
    if (client) {
        while (auto opt_packet = client->ReceivePacket()) {
            if (opt_packet->type == network::PacketType::kCharacterListResponse) {
                auto resp = network::CharacterListResponsePacket::Deserialize(opt_packet->payload);
                BuildUI(resp.characters);
                state_ = State::Loaded;
            } else if (opt_packet->type == network::PacketType::kSelectCharacterResponse) {
                auto resp = network::SelectCharacterResponsePacket::Deserialize(opt_packet->payload);
                if (resp.success) {
                    engine_->GetScreens().Push(std::make_unique<GameScreen>(engine_, true)); // Existing character
                } else {
                    // Handle error (show message)
                }
            }
        }
    }
}

}
'''
with open('d:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp', 'w') as f:
    f.write(content_cpp)

