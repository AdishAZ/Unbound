import re

with open('d:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp', 'r') as f:
    content = f.read()

# Add network include
content = content.replace('#include "ui/screens/create_account_screen.h"', '#include "ui/screens/create_account_screen.h"\n#include "network/multiplayer_client.h"\n#include "network/packet.h"')

# Update OnClick to send AuthRequest
click_search = '''    login_button_->OnClick([this]() {
        if (username_input_->GetText().empty() || password_input_->GetText().empty()) {
            status_label_->SetText(L("login.error.empty"));
            state_ = LoginState::Error;
            return;
        }
        state_ = LoginState::Connecting;
        state_timer_ = 0.0f;
        status_label_->SetText(L("login.connecting"));
        status_label_->SetColor(Color::DarkText);
        connect_progress_->SetProgress(0.0f);
    });'''
click_replace = '''    login_button_->OnClick([this]() {
        if (username_input_->GetText().empty() || password_input_->GetText().empty()) {
            status_label_->SetText(L("login.error.empty"));
            state_ = LoginState::Error;
            return;
        }
        
        auto client = engine_->GetNetworkClient();
        if (!client || !client->IsConnected()) {
            status_label_->SetText("Not connected to server!");
            state_ = LoginState::Error;
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
        connect_progress_->SetProgress(0.5f);
    });'''
content = content.replace(click_search, click_replace)

# Update Update() to poll for AuthResponse
update_search = '''void LoginScreen::Update(float dt) {
    if (root_layout_) {
        root_layout_->Update(dt);
    }

    if (state_ == LoginState::Connecting) {
        state_timer_ += dt;
        connect_progress_->SetProgress(state_timer_ / 1.0f);
        if (state_timer_ >= 1.0f) {
            state_ = LoginState::Authenticating;
            state_timer_ = 0.0f;
            status_label_->SetText(L("login.authenticating"));
            connect_progress_->SetProgress(0.5f);
        }
    } else if (state_ == LoginState::Authenticating) {
        state_timer_ += dt;
        connect_progress_->SetProgress(0.5f + (state_timer_ / 0.5f) * 0.5f);
        if (state_timer_ >= 0.5f) {
            state_ = LoginState::Connected;
            status_label_->SetText(L("login.connected"));
            connect_progress_->SetProgress(1.0f);
            
            engine_->GetScreens().Replace(std::make_unique<CharacterSelectScreen>(engine_));
        }
    }
}'''
update_replace = '''void LoginScreen::Update(float dt) {
    if (root_layout_) {
        root_layout_->Update(dt);
    }
    
    if (state_ == LoginState::Authenticating) {
        auto client = engine_->GetNetworkClient();
        if (client) {
            while (auto opt_packet = client->ReceivePacket()) {
                if (opt_packet->type == unboundmp::network::PacketType::kAuthResponse) {
                    auto resp = unboundmp::network::AuthResponsePacket::Deserialize(opt_packet->payload);
                    if (resp.success) {
                        state_ = LoginState::Connected;
                        status_label_->SetText(L("login.connected"));
                        connect_progress_->SetProgress(1.0f);
                        engine_->GetScreens().Replace(std::make_unique<CharacterSelectScreen>(engine_));
                    } else {
                        state_ = LoginState::Error;
                        status_label_->SetText(resp.message);
                        status_label_->SetColor(Color::DarkError);
                        connect_progress_->SetProgress(0.0f);
                    }
                }
            }
        }
    }
}'''
content = content.replace(update_search, update_replace)

with open('d:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp', 'w') as f:
    f.write(content)
