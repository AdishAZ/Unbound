#include "ui/screens/loading_screen.h"
#include "ui/ui_engine.h"
#include "ui/localization.h"
#include "ui/screens/game_screen.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"
#include "network/client_session_manager.h"
#include "emulator/game_bootstrap.h"

#include "core/log_manager.h"
#include "core/event_system.h"

namespace unboundmp::ui {

LoadingScreen::LoadingScreen(UIEngine* engine) : UIScreen("LoadingScreen"), engine_(engine), timer_(0.0f), step_(0) {
    
    root_ = std::make_shared<Container>();
    root_->SetBounds({0, 0, 800, 600});

    title_label_ = std::make_shared<Label>();
    title_label_->SetText(L("loading.connecting"));
    title_label_->SetBounds({200, 250, 400, 30});

    loading_bar_ = std::make_shared<ProgressBar>();
    loading_bar_->SetBounds({200, 300, 400, 20});

    status_label_ = std::make_shared<Label>();
    status_label_->SetBounds({200, 330, 400, 20});

    root_->AddChild(title_label_);
    root_->AddChild(loading_bar_);
    root_->AddChild(status_label_);
}

void LoadingScreen::OnEnter() {
    timer_ = 0.0f;
    step_ = 0;
    loading_bar_->SetProgress(0.0f);
    status_label_->SetText("Connecting...");

    auto& ev = core::EventSystem::GetInstance();
    subscriptions_.push_back(ev.Subscribe(core::EventType::kCharacterLoaded, [this](const core::Event& e) {
        auto loaded_ev = static_cast<const core::CharacterLoadedEvent&>(e);
        if (loaded_ev.success) {
            LOG_INFO(UI, "LoadingScreen: Success! Booting emulator.");
            loading_bar_->SetProgress(1.0f);
            status_label_->SetText("Entering World...");
            
            unboundmp::network::ClientSessionManager::GetInstance().SetSessionState(unboundmp::network::SessionState::kInGame);
            
            auto& bootstrap = unboundmp::emulator::GameBootstrap::GetInstance();
            bootstrap.Initialize();
            bootstrap.LoadSaveState();
            
            LOG_INFO(UI, "LoadingScreen: Transitioning to GameScreen.");
            engine_->GetScreens().Replace(std::make_unique<GameScreen>(engine_, true));
        } else {
            LOG_INFO(UI, "LoadingScreen: Failed response.");
            status_label_->SetText(loaded_ev.message);
            status_label_->SetColor(Color::DarkError);
        }
    }));
}

void LoadingScreen::OnExit() {
    auto& ev = core::EventSystem::GetInstance();
    for (auto id : subscriptions_) {
        ev.Unsubscribe(core::EventType::kCharacterLoaded, id);
    }
    subscriptions_.clear();
}
void LoadingScreen::OnPause() {}
void LoadingScreen::OnResume() {}

void LoadingScreen::Render(const RenderContext& ctx) {
    root_->Render(ctx);
}

bool LoadingScreen::HandleInput(const SDL_Event& event) {
    return root_->HandleInput(event);
}

void LoadingScreen::Update(float dt) {
    root_->Update(dt);
}

} // namespace unboundmp::ui
