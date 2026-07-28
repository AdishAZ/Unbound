#include "ui/notification_center.h"
#include "ui/widget.h"
#include "ui/animation.h"
#include <algorithm>
#include <format>

namespace unboundmp::ui {

std::string NotificationCenter::GenerateId() {
    return std::format("notif_{}", m_nextId++);
}

void NotificationCenter::ShowToast(const std::string& message, NotificationType type, float duration) {
    Notification notif;
    notif.id = GenerateId();
    notif.message = message;
    notif.type = type;
    notif.style = NotificationStyle::Toast;
    notif.duration = duration;
    notif.alpha = 0.0f; // Start faded out
    m_notifications.push_back(std::move(notif));
    
    if (anim_mgr_) {
        // Animate alpha from 0 to 1
        anim_mgr_->Add(std::make_unique<ProgressAnimation>(
            nullptr, 0.3f, &m_notifications.back().alpha, 0.0f, 1.0f, Easing::EaseOut
        ));
    }
}

void NotificationCenter::ShowDialog(const std::string& title, const std::string& message, ClickCallback on_confirm, ClickCallback on_cancel) {
    Notification notif;
    notif.id = GenerateId();
    notif.title = title;
    notif.message = message;
    notif.type = NotificationType::Info;
    notif.style = NotificationStyle::Dialog;
    notif.duration = 0.0f; // dialogs don't auto-dismiss
    notif.on_confirm = on_confirm;
    notif.on_cancel = on_cancel;
    
    // Dialogs go to front for priority
    m_notifications.insert(m_notifications.begin(), std::move(notif));
}

void NotificationCenter::ShowError(const std::string& title, const std::string& message) {
    ShowDialog(title, message, nullptr, nullptr);
}

void NotificationCenter::Dismiss(const std::string& id) {
    for (auto& notif : m_notifications) {
        if (notif.id == id && !notif.dismissing) {
            notif.dismissing = true;
            if (anim_mgr_) {
                anim_mgr_->Add(std::make_unique<ProgressAnimation>(
                    nullptr, 0.3f, &notif.alpha, notif.alpha, 0.0f, Easing::EaseIn
                ));
            } else {
                notif.alpha = 0.0f;
            }
        }
    }
}

void NotificationCenter::DismissAll() {
    for (auto& notif : m_notifications) {
        notif.dismissing = true;
    }
}

void NotificationCenter::Update(float dt) {
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        if (it->dismissing) {
            it->alpha -= dt * 5.0f; // fade out fast
            if (it->alpha <= 0.0f) {
                it = m_notifications.erase(it);
                continue;
            }
        } else {
            if (it->alpha < 1.0f) {
                it->alpha = std::min(1.0f, it->alpha + dt * 3.33f); // fade in 0.3s
            }
            
            if (it->duration > 0.0f) {
                it->elapsed += dt;
                if (it->elapsed >= it->duration) {
                    it->dismissing = true;
                }
            }
        }
        ++it;
    }
}

Color NotificationCenter::GetColorForType(NotificationType type) const {
    switch (type) {
        case NotificationType::Info: return {50, 150, 255, 255};
        case NotificationType::Success: return {50, 220, 50, 255};
        case NotificationType::Warning: return {255, 200, 50, 255};
        case NotificationType::Error: return {255, 50, 50, 255};
    }
    return {255, 255, 255, 255};
}

void NotificationCenter::Render(const RenderContext& ctx) {
    int toastYOffset = 20;
    
    // Reverse iterate to draw dialogs on top
    for (auto it = m_notifications.rbegin(); it != m_notifications.rend(); ++it) {
        if (it->style == NotificationStyle::Toast) {
            RenderToast(ctx, *it, toastYOffset);
            toastYOffset += 60; // spacing between toasts
        } else if (it->style == NotificationStyle::Dialog) {
            RenderDialog(ctx, *it);
        }
    }
}

void NotificationCenter::RenderToast(const RenderContext& ctx, const Notification& notif, int yOffset) {
    int width = 300;
    int height = 50;
    // Assuming 1280x720 logical size for now or we need screen size from context.
    int x = ctx.screen_width - width - 20; // Top right
    int y = yOffset;
    
    uint8_t a = static_cast<uint8_t>(255 * notif.alpha);
    
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, a);
    SDL_Rect bgRect{x, y, width, height};
    SDL_RenderFillRect(ctx.renderer, &bgRect);
    
    Color typeCol = GetColorForType(notif.type);
    SDL_SetRenderDrawColor(ctx.renderer, typeCol.r, typeCol.g, typeCol.b, a);
    SDL_Rect barRect{x, y, 4, height};
    SDL_RenderFillRect(ctx.renderer, &barRect);
    
    Widget::DrawText(ctx, notif.message, x + 10, y + 10, {255,255,255,a});
}

void NotificationCenter::RenderDialog(const RenderContext& ctx, const Notification& notif) {
    uint8_t a = static_cast<uint8_t>(200 * notif.alpha);
    
    // Overlay
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, a);
    SDL_Rect overlay{0, 0, ctx.screen_width, ctx.screen_height}; // Screen size bounds
    SDL_RenderFillRect(ctx.renderer, &overlay);
    
    // Dialog box
    int width = 400;
    int height = 200;
    int x = (ctx.screen_width - width) / 2;
    int y = (ctx.screen_height - height) / 2;
    
    SDL_SetRenderDrawColor(ctx.renderer, 40, 40, 40, static_cast<uint8_t>(255 * notif.alpha));
    SDL_Rect dialogRect{x, y, width, height};
    SDL_RenderFillRect(ctx.renderer, &dialogRect);
    
    Widget::DrawText(ctx, notif.title, x + 20, y + 20, {255,255,255,a});
    Widget::DrawText(ctx, notif.message, x + 20, y + 50, {200,200,200,a});
}

bool NotificationCenter::HandleInput(const SDL_Event& event) {
    if (!HasActiveDialog()) {
        return false;
    }
    
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        // Just dismiss the first dialog we find for now, in a real system we'd check bounds
        for (auto& notif : m_notifications) {
            if (notif.style == NotificationStyle::Dialog && !notif.dismissing) {
                if (notif.on_confirm) notif.on_confirm();
                notif.dismissing = true;
                return true;
            }
        }
    }
    return true; // block input
}

bool NotificationCenter::HasActiveDialog() const {
    for (const auto& notif : m_notifications) {
        if (notif.style == NotificationStyle::Dialog && !notif.dismissing) {
            return true;
        }
    }
    return false;
}

} // namespace unboundmp::ui
