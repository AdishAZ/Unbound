#include "ui/screens/character_creation_screen.h"
#include "ui/screens/game_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"
#include "network/client_session_manager.h"
#include "ui/screens/loading_screen.h"
#include <iostream>
#include "core/event_system.h"
#include "core/json.h"
#include "ui/theme.h"

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
    
    auto window = std::make_shared<Window>("char_create_win");
    window->SetTitle(L("NEW CHARACTER"));
    window->SetAnchor(AnchorPoint::Center);
    window->SetBounds({0, 0, 700, 550});
    
    auto main_layout = std::make_shared<HorizontalLayout>();
    main_layout->SetWidthPolicy(SizePolicy::Expand);
    main_layout->SetHeightPolicy(SizePolicy::Expand);
    main_layout->SetSpacing(20);
    
    // Left column: Customization Options
    auto left_col = std::make_shared<VerticalLayout>();
    left_col->SetBounds({0, 0, 320, 500});
    left_col->SetSpacing(10);
    
    auto add_option = [&left_col](const std::string& label_text, const std::vector<std::string>& options) -> std::shared_ptr<ComboBox> {
        auto row = std::make_shared<HorizontalLayout>();
        row->SetBounds({0, 0, 320, 30});
        
        auto lbl = std::make_shared<Label>();
        lbl->SetText(label_text);
        lbl->SetBounds({0, 0, 100, 30});
        lbl->SetAlignment(Alignment::Left);
        
        auto combo = std::make_shared<ComboBox>(label_text + "_combo");
        combo->SetBounds({0, 0, 200, 30});
        for (const auto& opt : options) {
            combo->AddItem(opt);
        }
        combo->SetSelectedIndex(0);
        
        row->AddChild(lbl);
        row->AddChild(combo);
        left_col->AddChild(row);
        return combo;
    };
    
    // Basic fields
    auto name_row = std::make_shared<HorizontalLayout>();
    name_row->SetBounds({0, 0, 320, 30});
    auto name_lbl = std::make_shared<Label>();
    name_lbl->SetText("Character Name");
    name_lbl->SetBounds({0, 0, 120, 30});
    name_lbl->SetAlignment(Alignment::Left);
    name_input_ = std::make_shared<TextBox>();
    name_input_->SetBounds({0, 0, 180, 30});
    name_input_->SetPlaceholder("Name...");
    name_row->AddChild(name_lbl);
    name_row->AddChild(name_input_);
    left_col->AddChild(name_row);
    
    // Dropdowns (mocking PokeMMO options)
    gender_combo_ = add_option("Gender:", {"Male", "Female"});
    skin_combo_ = add_option("Skin Tone:", {"Skin Tone #1", "Skin Tone #2", "Skin Tone #3"});
    hat_combo_ = add_option("Hat:", {"None", "Red Cap", "Beanie"});
    hair_combo_ = add_option("Hair:", {"Very Spiky", "Messy", "Short"});
    eyes_combo_ = add_option("Eyes:", {"Grey Eyes", "Blue Eyes", "Brown Eyes"});
    face_combo_ = add_option("Face:", {"None", "Glasses"});
    back_combo_ = add_option("Back:", {"None", "Backpack"});
    top_combo_ = add_option("Top:", {"Long Sleeve Top", "T-Shirt", "Jacket"});
    gloves_combo_ = add_option("Gloves:", {"None", "Fingerless"});
    shoes_combo_ = add_option("Shoes:", {"Shoes", "Boots", "Sneakers"});
    legs_combo_ = add_option("Legs:", {"Pants", "Shorts"});
    bike_combo_ = add_option("Bicycle:", {"Red Bicycle", "Blue Bicycle"});
    region_combo_ = add_option("Region:", {"Kanto", "Hoenn", "Sinnoh", "Unova"});
    
    status_label_ = std::make_shared<Label>();
    status_label_->SetBounds({0, 0, 320, 20});
    status_label_->SetColor(unboundmp::ui::Color::DarkError);
    left_col->AddChild(status_label_);
    
    // Right column: Preview & Button
    auto right_col = std::make_shared<VerticalLayout>();
    right_col->SetBounds({0, 0, 320, 500});
    right_col->SetSpacing(20);
    
    auto preview_panel = std::make_shared<Panel>();
    preview_panel->SetBounds({0, 0, 320, 350});
    preview_panel->SetBorderThickness(2);
    preview_panel->SetCornerRadius(8);
    
    auto preview_lbl = std::make_shared<Label>();
    preview_lbl->SetText("(Sprite Preview Placeholder)");
    preview_lbl->SetBounds({0, 150, 320, 30});
    preview_lbl->SetAlignment(Alignment::Center);
    preview_panel->AddChild(preview_lbl);
    
    auto btn_layout = std::make_shared<HorizontalLayout>();
    btn_layout->SetBounds({0, 0, 320, 40});
    btn_layout->SetSpacing(20);
    
    auto back_btn = std::make_shared<Button>();
    back_btn->SetText("CANCEL");
    back_btn->SetBounds({0, 0, 100, 40});
    back_btn->OnClick([this]() {
        engine_->GetScreens().Pop();
    });
    
    auto create_btn = std::make_shared<Button>();
    create_btn->SetText("CREATE CHARACTER");
    create_btn->SetBounds({0, 0, 200, 40});
    create_btn->OnClick([this]() {
        auto client = engine_->GetNetworkClient();
        if (client) {
            unboundmp::network::CreateCharacterRequestPacket req;
            req.name = name_input_->GetText();
            
            // Serialize appearance using string concat since our custom JSON isn't full nlohmann
            std::string app = "{";
            app += "\"gender\":\"" + gender_combo_->GetSelectedItem() + "\",";
            app += "\"skin\":\"" + skin_combo_->GetSelectedItem() + "\",";
            app += "\"hat\":\"" + hat_combo_->GetSelectedItem() + "\",";
            app += "\"hair\":\"" + hair_combo_->GetSelectedItem() + "\",";
            app += "\"eyes\":\"" + eyes_combo_->GetSelectedItem() + "\",";
            app += "\"face\":\"" + face_combo_->GetSelectedItem() + "\",";
            app += "\"back\":\"" + back_combo_->GetSelectedItem() + "\",";
            app += "\"top\":\"" + top_combo_->GetSelectedItem() + "\",";
            app += "\"gloves\":\"" + gloves_combo_->GetSelectedItem() + "\",";
            app += "\"shoes\":\"" + shoes_combo_->GetSelectedItem() + "\",";
            app += "\"legs\":\"" + legs_combo_->GetSelectedItem() + "\",";
            app += "\"bike\":\"" + bike_combo_->GetSelectedItem() + "\",";
            app += "\"region\":\"" + region_combo_->GetSelectedItem() + "\"";
            app += "}";
            
            req.appearance = app;
            
            unboundmp::network::Packet p;
            p.type = unboundmp::network::PacketType::kCreateCharacterRequest;
            p.session_token = unboundmp::network::ClientSessionManager::GetInstance().GetSession()->session_token;
            p.payload = req.Serialize();
            client->SendPacket(p);
            
            status_label_->SetText("Creating...");
            status_label_->SetColor(unboundmp::ui::Color::DarkText);
        }
    });
    
    btn_layout->AddChild(back_btn);
    btn_layout->AddChild(create_btn);
    
    right_col->AddChild(preview_panel);
    right_col->AddChild(btn_layout);
    
    main_layout->AddChild(left_col);
    main_layout->AddChild(right_col);
    
    window->AddChild(main_layout);
    
    anchor->AddChild(bg_img);
    anchor->AddChild(window);
    root_layout_ = anchor;
}

void CharacterCreationScreen::OnEnter() {
    auto& ev = core::EventSystem::GetInstance();
    
    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterCreated, [this](const core::Event& e) {
        auto created_ev = static_cast<const core::CharacterCreatedEvent&>(e);
        pending_character_id_ = created_ev.character_id;
        
        unboundmp::network::SelectCharacterRequestPacket req;
        req.character_id = created_ev.character_id;
        unboundmp::network::Packet p;
        p.type = unboundmp::network::PacketType::kSelectCharacterRequest;
        p.session_token = unboundmp::network::ClientSessionManager::GetInstance().GetSession()->session_token;
        p.payload = req.Serialize();
        auto client = engine_->GetNetworkClient();
        if (client) client->SendPacket(p);
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterCreationFailed, [this](const core::Event& e) {
        auto fail_ev = static_cast<const core::CharacterCreationFailedEvent&>(e);
        status_label_->SetText(fail_ev.message);
        status_label_->SetColor(unboundmp::ui::Color::DarkError);
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterSelected, [this](const core::Event&) {
        unboundmp::network::ClientSessionManager::GetInstance().SetActiveCharacter(pending_character_id_);
        engine_->GetScreens().Replace(std::make_unique<LoadingScreen>(engine_));
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterSelectFailed, [this](const core::Event& e) {
        auto fail_ev = static_cast<const core::CharacterSelectFailedEvent&>(e);
        status_label_->SetText(fail_ev.message);
        status_label_->SetColor(unboundmp::ui::Color::DarkError);
    }));
}

void CharacterCreationScreen::OnExit() {
    auto& ev = core::EventSystem::GetInstance();
    for (auto id : subscriptions_) {
        ev.Unsubscribe(core::EventType::kCharacterCreated, id);
        ev.Unsubscribe(core::EventType::kCharacterCreationFailed, id);
        ev.Unsubscribe(core::EventType::kCharacterSelected, id);
        ev.Unsubscribe(core::EventType::kCharacterSelectFailed, id);
    }
    subscriptions_.clear();
}
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
}

} // namespace unboundmp::ui
