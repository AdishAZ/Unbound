#include "ui/components/popup_menu.h"
#include <algorithm>

namespace unboundmp::ui {

PopupMenu::PopupMenu() {
    bounds_.width = 150; // Default width
    bounds_.height = 0;
}

void PopupMenu::AddItem(const std::string& text, std::function<void()> callback) {
    MenuItem item;
    item.text = text;
    item.callback = callback;
    items_.push_back(item);
    UpdateLayout();
}

void PopupMenu::ClearItems() {
    items_.clear();
    UpdateLayout();
}

void PopupMenu::UpdateLayout() {
    int current_y = bounds_.y;
    for (auto& item : items_) {
        item.bounds.x = bounds_.x;
        item.bounds.y = current_y;
        item.bounds.width = bounds_.width;
        item.bounds.height = 28;
        current_y += item.bounds.height;
    }
    bounds_.height = current_y - bounds_.y;
}

void PopupMenu::ShowAnchored(Widget* anchor, AnchorPoint point) {
    if (!anchor) return;
    
    visible_ = true;
    Rect anchor_bounds = anchor->GetBounds(); // Assuming Widget has GetBounds() returning Rect
    
    switch (point) {
        case AnchorPoint::TopLeft:
            bounds_.x = anchor_bounds.x;
            bounds_.y = anchor_bounds.y - bounds_.height;
            break;
        case AnchorPoint::TopRight:
            bounds_.x = anchor_bounds.x + anchor_bounds.width - bounds_.width;
            bounds_.y = anchor_bounds.y - bounds_.height;
            break;
        case AnchorPoint::BottomLeft:
            bounds_.x = anchor_bounds.x;
            bounds_.y = anchor_bounds.y + anchor_bounds.height;
            break;
        case AnchorPoint::BottomRight:
            bounds_.x = anchor_bounds.x + anchor_bounds.width - bounds_.width;
            bounds_.y = anchor_bounds.y + anchor_bounds.height;
            break;
    }
    
    UpdateLayout();
}

void PopupMenu::Hide() {
    visible_ = false;
    hover_index_ = -1;
}

bool PopupMenu::IsVisible() const {
    return visible_;
}

void PopupMenu::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Clamp to screen
    int win_w = 800, win_h = 600;
    bounds_.x = std::max<int>(0, std::min<int>(bounds_.x, win_w - bounds_.width));
    bounds_.y = std::max<int>(0, std::min<int>(bounds_.y, win_h - bounds_.height));
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
        if (static_cast<int>(i) == hover_index_) {
            SDL_SetRenderDrawColor(ctx.renderer, 70, 70, 70, 255);
            SDL_Rect hover_rect{item.bounds.x + 1, item.bounds.y + 1, item.bounds.width - 2, item.bounds.height - 2};
            SDL_RenderFillRect(ctx.renderer, &hover_rect);
        }
        Color text_color = {255, 255, 255, 255};
        DrawText(ctx, item.text, item.bounds.x + 8, item.bounds.y + 6, text_color);
    }
}

bool PopupMenu::HandleInput(const SDL_Event& event) {
    if (!visible_) return false;

    if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;
        hover_index_ = -1;
        
        for (size_t i = 0; i < items_.size(); ++i) {
            if (mx >= items_[i].bounds.x && mx <= items_[i].bounds.x + items_[i].bounds.width &&
                my >= items_[i].bounds.y && my <= items_[i].bounds.y + items_[i].bounds.height) {
                hover_index_ = static_cast<int>(i);
                break;
            }
        }
        return true; 
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;
        
        bool clicked_inside = (mx >= bounds_.x && mx <= bounds_.x + bounds_.width &&
                               my >= bounds_.y && my <= bounds_.y + bounds_.height);
                               
        if (clicked_inside) {
            for (size_t i = 0; i < items_.size(); ++i) {
                if (mx >= items_[i].bounds.x && mx <= items_[i].bounds.x + items_[i].bounds.width &&
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
            return false; 
        }
    }
    
    return false;
}

} // namespace unboundmp::ui
