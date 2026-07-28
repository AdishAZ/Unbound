#include "ui/window_manager.h"
#include <algorithm>

namespace unboundmp::ui {

static WindowManager* g_wm = nullptr;

WindowManager::WindowManager() { g_wm = this; }

WindowManager* WindowManager::GetInstance() { return g_wm; }

void WindowManager::AddWindow(std::shared_ptr<Window> window) {
    windows_.push_back(window);
}

void WindowManager::RemoveWindow(const std::string& id) {
    windows_.erase(std::remove_if(windows_.begin(), windows_.end(),
        [&id](const std::shared_ptr<Window>& w) { return w->GetId() == id; }), windows_.end());
}

void WindowManager::RemoveWindow(Window* window) {
    windows_.erase(std::remove_if(windows_.begin(), windows_.end(),
        [window](const std::shared_ptr<Window>& w) { return w.get() == window; }), windows_.end());
}

void WindowManager::BringToFront(Window* window) {
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [window](const std::shared_ptr<Window>& w) { return w.get() == window; });
    if (it != windows_.end() && it != windows_.end() - 1) {
        auto w = *it;
        windows_.erase(it);
        windows_.push_back(w);
    }
}

void WindowManager::SendToBack(Window* window) {
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [window](const std::shared_ptr<Window>& w) { return w.get() == window; });
    if (it != windows_.end() && it != windows_.begin()) {
        auto w = *it;
        windows_.erase(it);
        windows_.insert(windows_.begin(), w);
    }
}

void WindowManager::SetModal(std::shared_ptr<Window> window) {
    modal_window_ = window;
}

void WindowManager::ClearModal() {
    modal_window_.reset();
}

void WindowManager::Update(float dt) {
    for (auto& w : windows_) {
        w->Update(dt);
    }
    if (modal_window_) {
        modal_window_->Update(dt);
    }
    for (auto& p : popups_) p->Update(dt);
}

void WindowManager::Render(const RenderContext& ctx) {
    for (auto& w : windows_) {
        if (w->IsVisible()) w->Render(ctx);
    }
    
    if (modal_window_ && modal_window_->IsVisible()) {
        // Draw dark overlay
        ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {0, 0, 0, 150});
        modal_window_->Render(ctx);
    }
    
    for (auto& p : popups_) {
        if (p->IsVisible()) p->Render(ctx);
    }
}

bool WindowManager::HandleInput(const SDL_Event& event) {
    if (!popups_.empty()) {
        if (popups_.back()->HandleInput(event)) return true;
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            ClosePopup(); // Click outside popup closes it
            return true;
        }
    }

    if (modal_window_ && modal_window_->IsVisible()) {
        if (modal_window_->HandleInput(event)) return true;
        
        // Modal absorbs all mouse clicks
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION) {
            return true; 
        }
        return false;
    }

    // Input handled from top to bottom
    for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
        if (!(*it)->IsVisible()) continue;
        
        if ((*it)->HandleInput(event)) {
            // If window was clicked, bring to front
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                BringToFront(it->get());
            }
            return true;
        }
    }
    
    return false;
}


void WindowManager::ShowPopup(std::shared_ptr<Widget> popup, int x, int y) {
    popup->SetBounds({x, y, popup->GetBounds().width, popup->GetBounds().height});
    popups_.clear(); // Only one popup at a time
    popups_.push_back(popup);
}

void WindowManager::ClosePopup() {
    popups_.clear();
}

} // namespace unboundmp::ui
