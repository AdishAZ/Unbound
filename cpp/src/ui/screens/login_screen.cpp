#include "ui/screens/login_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "ui/screens/character_select_screen.h"
#include "ui/screens/create_account_screen.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"
#include "network/client_session_manager.h"
#include "core/event_system.h"

namespace unboundmp::ui {

LoginScreen::LoginScreen(UIEngine* engine) : UIScreen("LoginScreen"), engine_(engine), state_(LoginState::Idle), state_timer_(0.0f) {
    auto anchor_layout = std::make_shared<AnchorLayout>();
    anchor_layout->SetBounds({0, 0, engine->GetRenderContext().screen_width, engine->GetRenderContext().screen_height});
    anchor_layout->SetWidthPolicy(SizePolicy::Expand);
    anchor_layout->SetHeightPolicy(SizePolicy::Expand);
    
    auto bg_img = std::make_shared<ImageWidget>("bg");
    bg_img->SetWidthPolicy(SizePolicy::Expand);
    bg_img->SetHeightPolicy(SizePolicy::Expand);
    SDL_Texture* tex = engine->GetAssetManager().LoadTexture("login_bg", "assets/ui/login_bg.bmp");
    if (tex) bg_img->SetTexture(tex);
    anchor_layout->AddChild(bg_img);
    
    panel_ = std::make_shared<Panel>("login_panel");
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 400, 480}); // Fixed size panel
    panel_->SetBackgroundColor(Color::DarkPanel);
    panel_->SetCornerRadius(8);
    // panel_->SetPadding(Padding::All(20));
    
    auto vert_layout = std::make_shared<VerticalLayout>("login_vert");
    vert_layout->SetBounds({0, 0, 400, 480});
    vert_layout->SetPadding(Padding::All(20));
    vert_layout->SetSpacing(15);
    
    // Logo or Title
    title_label_ = std::make_shared<Label>();
    title_label_->SetText(L("login.title"));
    title_label_->SetAlignment(Alignment::Center);
    title_label_->SetHeightPolicy(SizePolicy::Fixed);
    title_label_->SetBounds({0, 0, 360, 30});
    // title_label_->SetColor(Color::DarkAccent);
    
    auto server_label = std::make_shared<Label>();
    server_label->SetText("Select Server");
    server_label->SetBounds({0, 0, 360, 20});
    
    auto server_combo = std::make_shared<ComboBox>("server_select");
    server_combo->SetBounds({0, 0, 360, 35});
    server_combo->AddItem("Global [NA]");
    server_combo->AddItem("Global [EU]");
    server_combo->AddItem("Test Realm");
    server_combo->SetSelectedIndex(0);
    
    user_label_ = std::make_shared<Label>();
    user_label_->SetText("Username");
    user_label_->SetBounds({0, 0, 360, 20});
    
    username_input_ = std::make_shared<TextBox>();
    username_input_->SetBounds({0, 0, 360, 35});
    username_input_->SetPlaceholder("Enter username...");
    username_input_->SetTabIndex(1);
    
    pass_label_ = std::make_shared<Label>();
    pass_label_->SetText("Password");
    pass_label_->SetBounds({0, 0, 360, 20});
    
    password_input_ = std::make_shared<TextBox>();
    password_input_->SetBounds({0, 0, 360, 35});
    password_input_->SetPassword(true);
    password_input_->SetPlaceholder("Enter password...");
    password_input_->SetTabIndex(2);
    
    remember_checkbox_ = std::make_shared<Checkbox>();
    remember_checkbox_->SetLabel(L("login.remember"));
    remember_checkbox_->SetBounds({0, 0, 360, 25});
    remember_checkbox_->SetTabIndex(3);
    
    login_button_ = std::make_shared<Button>();
    login_button_->SetText(L("login.button"));
    login_button_->SetBounds({0, 0, 360, 40});
    login_button_->SetTabIndex(4);
    
    status_label_ = std::make_shared<Label>();
    status_label_->SetBounds({0, 0, 360, 20});
    status_label_->SetAlignment(Alignment::Center);
    status_label_->SetColor(Color::DarkError);
    
    connect_progress_ = std::make_shared<ProgressBar>();
    connect_progress_->SetBounds({0, 0, 360, 10});
    
    auto links_layout = std::make_shared<HorizontalLayout>();
    links_layout->SetBounds({0, 0, 360, 25});
    links_layout->SetSpacing(10);
    
    auto create_acc_btn = std::make_shared<Button>();
    create_acc_btn->SetText("Create Account");
    create_acc_btn->SetBounds({0, 0, 175, 25});
    create_acc_btn->SetTabIndex(5);
    create_acc_btn->OnClick([this]() {
        engine_->GetScreens().Replace(std::make_unique<CreateAccountScreen>(engine_));
    });
    
    auto forgot_pwd_btn = std::make_shared<Button>();
    forgot_pwd_btn->SetText("Forgot Password");
    forgot_pwd_btn->SetBounds({0, 0, 175, 25});
    forgot_pwd_btn->SetTabIndex(6);
    
    links_layout->AddChild(create_acc_btn);
    links_layout->AddChild(forgot_pwd_btn);
    
    vert_layout->AddChild(title_label_);
    vert_layout->AddChild(server_label);
    vert_layout->AddChild(server_combo);
    vert_layout->AddChild(user_label_);
    vert_layout->AddChild(username_input_);
    vert_layout->AddChild(pass_label_);
    vert_layout->AddChild(password_input_);
    vert_layout->AddChild(remember_checkbox_);
    vert_layout->AddChild(login_button_);
    vert_layout->AddChild(links_layout);
    vert_layout->AddChild(status_label_);
    vert_layout->AddChild(connect_progress_);
    
    panel_->AddChild(vert_layout);
    anchor_layout->AddChild(panel_);
    
    // Store root layout
    root_layout_ = anchor_layout;
    
    login_button_->OnClick([this]() {
        if (username_input_->GetText().empty() || password_input_->GetText().empty()) {
            status_label_->SetText(L("login.error.empty"));
            state_ = LoginState::Error;
            return;
        }
        
        auto client = engine_->GetNetworkClient();
        if (!client) {
            status_label_->SetText("Network subsystem unavailable!");
            state_ = LoginState::Error;
            return;
        }
        
        if (!client->IsConnected()) {
            client->Reconnect();
            state_ = LoginState::Connecting;
            status_label_->SetText("Connecting to server...");
            status_label_->SetColor(Color::DarkText);
            connect_progress_->SetVisible(true);
            connect_progress_->SetProgress(0.1f);
            state_timer_ = 0.0f;
            return;
        }
        
        unboundmp::network::AuthRequestPacket auth_req;
        auth_req.username = username_input_->GetText();
        auth_req.password = password_input_->GetText();
        
        unboundmp::network::Packet p;
        p.type = unboundmp::network::PacketType::kAuthRequest;
        p.payload = auth_req.Serialize();
        client->SendPacket(p);
        
        state_ = LoginState::Authenticating;
        status_label_->SetText(L("login.authenticating"));
        status_label_->SetColor(Color::DarkText);
        connect_progress_->SetVisible(true);
        connect_progress_->SetProgress(0.5f);
    });
}

void LoginScreen::OnEnter() {
    state_ = LoginState::Idle;
    status_label_->SetText("");
    connect_progress_->SetVisible(false);
    connect_progress_->SetProgress(0.0f);
    
    if (remember_checkbox_->IsChecked()) {
        username_input_->SetText("Player1");
    }

    auto& ev = core::EventSystem::GetInstance();
    subscriptions_.push_back(ev.Subscribe(core::EventType::kLoginSuccess, [this](const core::Event&) {
        if (state_ == LoginState::Authenticating) {
            state_ = LoginState::Connected;
            status_label_->SetText(L("login.connected"));
            connect_progress_->SetProgress(1.0f);
            engine_->GetScreens().Replace(std::make_unique<CharacterSelectScreen>(engine_));
        }
    }));

    subscriptions_.push_back(ev.Subscribe(core::EventType::kLoginFailed, [this](const core::Event& e) {
        if (state_ == LoginState::Authenticating) {
            auto fail_ev = static_cast<const core::LoginFailedEvent&>(e);
            state_ = LoginState::Error;
            status_label_->SetText(fail_ev.message);
            status_label_->SetColor(Color::DarkError);
            connect_progress_->SetVisible(false);
            connect_progress_->SetProgress(0.0f);
        }
    }));
}

void LoginScreen::OnExit() {
    auto& ev = core::EventSystem::GetInstance();
    for (auto id : subscriptions_) {
        ev.Unsubscribe(core::EventType::kLoginSuccess, id);
        ev.Unsubscribe(core::EventType::kLoginFailed, id);
    }
    subscriptions_.clear();
}
void LoginScreen::OnPause() {}
void LoginScreen::OnResume() {}

void LoginScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void LoginScreen::Render(const RenderContext& ctx) {
    if (root_layout_) {
        // Force the root layout to take the full screen size
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool LoginScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) {
        return root_layout_->HandleInput(event);
    }
    return false;
}

void LoginScreen::Update(float dt) {
    if (root_layout_) {
        root_layout_->Update(dt);
    }
    
    if (state_ == LoginState::Connecting) {
        state_timer_ += dt;
        auto client = engine_->GetNetworkClient();
        if (client && client->IsConnected()) {
            unboundmp::network::AuthRequestPacket auth_req;
            auth_req.username = username_input_->GetText();
            auth_req.password = password_input_->GetText();
            
            unboundmp::network::Packet p;
            p.type = unboundmp::network::PacketType::kAuthRequest;
            p.payload = auth_req.Serialize();
            client->SendPacket(p);
            
            state_ = LoginState::Authenticating;
            status_label_->SetText(L("login.authenticating"));
            status_label_->SetColor(Color::DarkText);
            connect_progress_->SetVisible(true);
            connect_progress_->SetProgress(0.5f);
        } else if (state_timer_ > 3.0f) {
            state_ = LoginState::Error;
            status_label_->SetText("Connection failed!");
            status_label_->SetColor(Color::DarkError);
            connect_progress_->SetVisible(false);
        }
    }
}

} // namespace unboundmp::ui
