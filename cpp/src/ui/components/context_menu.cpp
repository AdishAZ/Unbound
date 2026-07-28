#include "ui/components/context_menu.h"
#include <algorithm>

namespace unboundmp::ui {

ContextMenu::ContextMenu() {
    bounds_.width = 150; // Default width
    bounds_.height = 0;
}

void ContextMenu::AddItem(const std::string& text, std::function<void()> callback) {
    MenuItem item;
    item.text = text;
    item.callback = callback;
    item.is_separator = false;
    items_.push_back(item);
    UpdateLayout();
}

void ContextMenu::AddSeparator() {
    MenuItem item;
    item.is_separator = true;
    items_.push_back(item);
    UpdateLayout();
}

void ContextMenu::ClearItems() {
    items_.clear();
    UpdateLayout();
}

void ContextMenu::Update(float dt) {}

void ContextMenu::UpdateLayout() {
    int current_y = bounds_.y;
    for (auto& item : items_) {
        item.bounds.x = bounds_.x;
        item.bounds.y = current_y;
        item.bounds.width = bounds_.width;
        item.bounds.height = item.is_separator ? 4 : 28;
        current_y += item.bounds.height;
    }
    bounds_.height = current_y - bounds_.y;
}

void ContextMenu::Show(int x, int y) {
    visible_ = true;
    bounds_.x = x;
    bounds_.y = y;
    
    // Clamp to screen bounds (assuming generic 800x600 for now, though real implementation might query RenderContext)
    // Here we'll do a basic clamp if bounds are known, but we don't have screen size here without context.
    // For safety, we keep it simple. If we had screen bounds, we would clamp bounds_.x and bounds_.y.
    
    UpdateLayout();
}

void ContextMenu::Hide() {
    visible_ = false;
    hover_index_ = -1;
}

bool ContextMenu::IsVisible() const {
    return visible_;
}

void ContextMenu::Render(const RenderContext& ctx) {
    if (!visible_) return;

    // Optional: clamp bounds to screen based on context window size if available.
    int win_w = 800, win_h = 600;
    if (bounds_.x + bounds_.width > win_w) bounds_.x = std::max(0, win_w - bounds_.width);
    if (bounds_.y + bounds_.height > win_h) bounds_.y = std::max(0, win_h - bounds_.height);
    UpdateLayout();

    // Background
    SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 30, 240);
    SDL_Rect bg_rect{bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &bg_rect);

    // Border
    SDL_SetRenderDrawColor(ctx.renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(ctx.renderer, &bg_rect);

    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        if (item.is_separator) {
            SDL_SetRenderDrawColor(ctx.renderer, 100, 100, 100, 255);
            SDL_RenderDrawLine(ctx.renderer, item.bounds.x + 4, item.bounds.y + 2, item.bounds.x + item.bounds.width - 4, item.bounds.y + 2);
        } else {
            if (static_cast<int>(i) == hover_index_) {
                SDL_SetRenderDrawColor(ctx.renderer, 70, 70, 70, 255);
                SDL_Rect hover_rect{item.bounds.x + 1, item.bounds.y + 1, item.bounds.width - 2, item.bounds.height - 2};
                SDL_RenderFillRect(ctx.renderer, &hover_rect);
            }
            Color text_color = {255, 255, 255, 255};
            DrawText(ctx, item.text, item.bounds.x + 8, item.bounds.y + 6, text_color);
        }
    }
}

bool ContextMenu::HandleInput(const SDL_Event& event) {
    if (!visible_) return false;

    if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;
        hover_index_ = -1;
        
        for (size_t i = 0; i < items_.size(); ++i) {
            if (!items_[i].is_separator &&
                mx >= items_[i].bounds.x && mx <= items_[i].bounds.x + items_[i].bounds.width &&
                my >= items_[i].bounds.y && my <= items_[i].bounds.y + items_[i].bounds.height) {
                hover_index_ = static_cast<int>(i);
                break;
            }
        }
        return true; // consume motion when visible
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;
        
        bool clicked_inside = (mx >= bounds_.x && mx <= bounds_.x + bounds_.width &&
                               my >= bounds_.y && my <= bounds_.y + bounds_.height);
                               
        if (clicked_inside) {
            for (size_t i = 0; i < items_.size(); ++i) {
                if (!items_[i].is_separator &&
                    mx >= items_[i].bounds.x && mx <= items_[i].bounds.x + items_[i].bounds.width &&
                    my >= items_[i].bounds.y && my <= items_[i].bounds.y + items_[i].bounds.height) {
                    if (items_[i].callback) {
                        items_[i].callback();
                    }
                    Hide();
                    break;
                }
            }
            return true;
        } else {
            Hide();
            return false; // let other widgets handle the click
        }
    }
    
    return false;
}

} // namespace unboundmp::ui
