#include "ui/screens/character_select_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/screens/character_creation_screen.h"
#include "ui/screens/loading_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "network/multiplayer_client.h"
#include "network/client_session_manager.h"
#include "core/log_manager.h"
#include "core/event_system.h"
#include "persistence/client_save_manager.h"

namespace unboundmp::ui {

CharacterSelectScreen::CharacterSelectScreen(UIEngine* engine) : UIScreen("CharacterSelectScreen"), engine_(engine), status_label_(nullptr) {
    root_layout_ = std::make_shared<AnchorLayout>();
    root_layout_->SetWidthPolicy(SizePolicy::Expand);
    root_layout_->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex);
    
    auto window = std::make_shared<Window>("char_select_win");
    window->SetTitle(L("char_select.title"));
    window->SetAnchor(AnchorPoint::Center);
    // Fixed window size for character list
    window->SetBounds({0, 0, 600, 400});
    
    auto horiz = std::make_shared<HorizontalLayout>();
    horiz->SetWidthPolicy(SizePolicy::Expand);
    horiz->SetHeightPolicy(SizePolicy::Expand);
    horiz->SetPadding(Padding::All(10));
    horiz->SetSpacing(15);
    
    // Left side: Character list
    chars_layout_ = std::make_shared<VerticalLayout>();
    chars_layout_->SetWidthPolicy(SizePolicy::Expand);
    chars_layout_->SetHeightPolicy(SizePolicy::Expand);
    chars_layout_->SetSpacing(10);
    
    // Right side: Action buttons
    auto btn_layout = std::make_shared<VerticalLayout>();
    btn_layout->SetWidthPolicy(SizePolicy::Fixed);
    btn_layout->SetHeightPolicy(SizePolicy::Expand);
    btn_layout->SetBounds({0, 0, 200, 0}); // Fixed width of 200
    btn_layout->SetSpacing(10);
    
    status_label_ = std::make_shared<Label>();
    status_label_->SetText("");
    status_label_->SetWidthPolicy(SizePolicy::Expand);
    status_label_->SetHeightPolicy(SizePolicy::Fixed);
    status_label_->SetBounds({0, 0, 0, 20});
    status_label_->SetAlignment(Alignment::Center);
    btn_layout->AddChild(status_label_);
    
    auto new_btn = std::make_shared<Button>();
    new_btn->SetText("NEW CHARACTER");
    new_btn->SetWidthPolicy(SizePolicy::Expand);
    new_btn->SetHeightPolicy(SizePolicy::Fixed);
    new_btn->SetBounds({0, 0, 0, 35});
    new_btn->OnClick([this]() {
        engine_->GetScreens().Push(std::make_unique<CharacterCreationScreen>(engine_));
    });
    
    // Delete button removed from here, it will be on the individual cards

    
    auto logout_btn = std::make_shared<Button>();
    logout_btn->SetText("LOGOUT");
    logout_btn->SetWidthPolicy(SizePolicy::Expand);
    logout_btn->SetHeightPolicy(SizePolicy::Fixed);
    logout_btn->SetBounds({0, 0, 0, 35});
    logout_btn->OnClick([this]() {
        core::EventSystem::GetInstance().Publish(core::EventType::kReturnToLoginRequested, core::EmptyEvent());
    });
    
    btn_layout->AddChild(new_btn);
    btn_layout->AddChild(logout_btn);
    
    horiz->AddChild(chars_layout_);
    horiz->AddChild(btn_layout);
    
    window->AddChild(horiz);
    
    root_layout_->AddChild(bg_img);
    root_layout_->AddChild(window);
}

void CharacterSelectScreen::BuildUI(const std::vector<network::CharacterEntry>& characters) {
    chars_layout_->ClearChildren();
    
    for (const auto& c : characters) {
        auto card = std::make_shared<Panel>();
        card->SetWidthPolicy(SizePolicy::Expand);
        card->SetHeightPolicy(SizePolicy::Fixed);
        card->SetBounds({0, 0, 0, 60}); // Height 60
        card->SetBorderThickness(2);
        card->SetCornerRadius(8);
        
        auto name = std::make_shared<Label>();
        name->SetText(c.name);
        name->SetBounds({10, 15, 180, 30});
        name->SetAlignment(Alignment::Left);
        
        auto btn = std::make_shared<Button>();
        btn->SetText("Play");
        btn->SetBounds({200, 10, 80, 40});
        
        auto del_btn = std::make_shared<Button>();
        del_btn->SetText("Del");
        del_btn->SetBounds({290, 10, 50, 40});
        
        uint64_t char_id = c.id;
        std::string char_name = c.name;
        btn->OnClick([this, char_id, char_name]() {
            pending_character_id_ = char_id;
            pending_character_name_ = char_name;
            auto client = engine_->GetNetworkClient();
            if (client) {
                network::SelectCharacterRequestPacket req;
                req.character_id = char_id;
                network::Packet p;
                p.type = network::PacketType::kSelectCharacterRequest;
                p.session_token = unboundmp::network::ClientSessionManager::GetInstance().GetSession()->session_token;
                p.payload = req.Serialize();
                client->SendPacket(p);
            }
        });
        
        del_btn->OnClick([this, char_id]() {
            auto client = engine_->GetNetworkClient();
            if (client) {
                network::DeleteCharacterRequestPacket req;
                req.character_id = char_id;
                network::Packet p;
                p.type = network::PacketType::kDeleteCharacterRequest;
                p.session_token = unboundmp::network::ClientSessionManager::GetInstance().GetSession()->session_token;
                p.payload = req.Serialize();
                client->SendPacket(p);
                
                // Refresh list immediately
                BuildCharacterList();
            }
        });
        
        card->AddChild(name);
        card->AddChild(btn);
        card->AddChild(del_btn);
        chars_layout_->AddChild(card);
    }
}

void CharacterSelectScreen::BuildCharacterList() {
    auto client = engine_->GetNetworkClient();
    if (client) {
        network::CharacterListRequestPacket req;
        network::Packet p;
        p.type = network::PacketType::kCharacterListRequest;
        p.session_token = unboundmp::network::ClientSessionManager::GetInstance().GetSession()->session_token;
        p.payload = req.Serialize();
        client->SendPacket(p);
        state_ = State::Fetching;
    }
}

void CharacterSelectScreen::OnEnter() {
    auto& ev = core::EventSystem::GetInstance();
    
    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterListReceived, [this](const core::Event& e) {
        auto ev_list = static_cast<const core::CharacterListEvent&>(e);
        BuildUI(ev_list.characters);
        state_ = State::Loaded;
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterSelected, [this](const core::Event&) {
        unboundmp::network::ClientSessionManager::GetInstance().SetActiveCharacter(pending_character_id_);
        
        auto session = unboundmp::network::ClientSessionManager::GetInstance().GetSession();
        if (session) {
            unboundmp::persistence::ClientSaveManager::GetInstance().SetActiveCharacter(session->account_id, pending_character_id_, pending_character_name_);
        }
        
        engine_->GetScreens().Replace(std::make_unique<LoadingScreen>(engine_));
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterSelectFailed, [this](const core::Event& e) {
        auto fail_ev = static_cast<const core::CharacterSelectFailedEvent&>(e);
        status_label_->SetText(fail_ev.message);
    }));

    BuildCharacterList();
}

void CharacterSelectScreen::OnExit() {
    auto& ev = core::EventSystem::GetInstance();
    for (auto id : subscriptions_) {
        ev.Unsubscribe(core::EventType::kCharacterListReceived, id);
        ev.Unsubscribe(core::EventType::kCharacterSelected, id);
        ev.Unsubscribe(core::EventType::kCharacterSelectFailed, id);
    }
    subscriptions_.clear();
}
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
    if (root_layout_) {
        return root_layout_->HandleInput(event);
    }
    return false;
}

void CharacterSelectScreen::Update(float dt) {
    if (root_layout_) {
        root_layout_->Update(dt);
    }
}

} // namespace unboundmp::ui
