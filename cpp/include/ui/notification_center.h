#pragma once

#include "ui/ui_types.h"
#include <string>
#include <vector>
#include <functional>
#include <SDL2/SDL.h>

namespace unboundmp::ui {

class AnimationManager;

enum class NotificationType { Info, Success, Warning, Error };
enum class NotificationStyle { Toast, Dialog };

using ClickCallback = std::function<void()>;

struct Notification {
    std::string id;
    std::string title;
    std::string message;
    NotificationType type = NotificationType::Info;
    NotificationStyle style = NotificationStyle::Toast;
    float duration = 3.0f; // seconds, 0 = permanent until dismissed
    int priority = 0; // higher = shown first
    ClickCallback on_confirm;
    ClickCallback on_cancel;
    float elapsed = 0.0f;
    float alpha = 1.0f;
    bool dismissing = false;
};

class NotificationCenter {
public:
    void SetAnimationManager(AnimationManager* anim_mgr) { anim_mgr_ = anim_mgr; }

    void ShowToast(const std::string& message, NotificationType type = NotificationType::Info, float duration = 3.0f);
    void ShowDialog(const std::string& title, const std::string& message, ClickCallback on_confirm, ClickCallback on_cancel = nullptr);
    void ShowError(const std::string& title, const std::string& message);
    
    void Dismiss(const std::string& id);
    void DismissAll();
    
    void Update(float dt);
    void Render(const RenderContext& ctx);
    bool HandleInput(const SDL_Event& event);
    
    bool HasActiveDialog() const;

private:
    std::vector<Notification> m_notifications;
    int m_nextId = 1;
    AnimationManager* anim_mgr_ = nullptr;
    
    std::string GenerateId();
    void RenderToast(const RenderContext& ctx, const Notification& notif, int yOffset);
    void RenderDialog(const RenderContext& ctx, const Notification& notif);
    Color GetColorForType(NotificationType type) const;
};

} // namespace unboundmp::ui
