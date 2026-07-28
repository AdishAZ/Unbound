#include "ui/screen_manager.h"

namespace unboundmp::ui {

void ScreenManager::Push(std::unique_ptr<UIScreen> screen) {
    if (!screens_.empty()) {
        screens_.back()->OnPause();
    }
    screen->OnEnter();
    screens_.push_back(std::move(screen));
}

void ScreenManager::Pop() {
    if (screens_.empty()) return;
    
    screens_.back()->OnExit();
    screens_.pop_back();
    
    if (!screens_.empty()) {
        screens_.back()->OnResume();
    }
}

void ScreenManager::Replace(std::unique_ptr<UIScreen> screen) {
    if (!screens_.empty()) {
        screens_.back()->OnExit();
        screens_.pop_back();
    }
    screen->OnEnter();
    screens_.push_back(std::move(screen));
}

void ScreenManager::Overlay(std::unique_ptr<UIScreen> screen) {
    screen->SetOverlay(true);
    screen->OnEnter();
    screens_.push_back(std::move(screen));
}

void ScreenManager::Render(const RenderContext& ctx) {
    if (screens_.empty()) return;

    size_t start_index = 0;
    for (int i = static_cast<int>(screens_.size()) - 1; i >= 0; --i) {
        if (!screens_[i]->IsOverlay()) {
            start_index = i;
            break;
        }
    }

    for (size_t i = start_index; i < screens_.size(); ++i) {
        screens_[i]->Render(ctx);
    }
}

bool ScreenManager::HandleInput(const SDL_Event& event) {
    if (screens_.empty()) return false;
    return screens_.back()->HandleInput(event);
}

void ScreenManager::Update(float dt) {
    if (screens_.empty()) return;

    size_t start_index = 0;
    for (int i = static_cast<int>(screens_.size()) - 1; i >= 0; --i) {
        if (!screens_[i]->IsOverlay()) {
            start_index = i;
            break;
        }
    }

    for (size_t i = start_index; i < screens_.size(); ++i) {
        screens_[i]->Update(dt);
    }
}

UIScreen* ScreenManager::GetCurrentScreen() const {
    if (screens_.empty()) return nullptr;
    return screens_.back().get();
}

} // namespace unboundmp::ui
