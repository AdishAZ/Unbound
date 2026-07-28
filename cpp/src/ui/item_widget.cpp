#include "ui/item_widget.h"
#include "ui/theme.h"
#include <SDL2/SDL.h>

namespace unboundmp::ui {

ItemWidget::ItemWidget(const std::string& id) : Widget(id) {
    bounds_.width = 240; // Wider for item name and quantity
    bounds_.height = 56; // Standard card height
}

void ItemWidget::Render(const RenderContext& ctx) {
    SDL_Renderer* renderer = ctx.renderer;
    
    // Draw background
    Color current_bg = selected_ ? selected_color_ : (hover_ ? hover_color_ : bg_color_);
    SDL_SetRenderDrawColor(renderer, current_bg.r, current_bg.g, current_bg.b, current_bg.a);
    SDL_Rect bg_rect = { bounds_.x, bounds_.y, bounds_.width, bounds_.height };
    SDL_RenderFillRect(renderer, &bg_rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, border_color_.r, border_color_.g, border_color_.b, border_color_.a);
    SDL_RenderDrawRect(renderer, &bg_rect);
    
    // Draw Icon placeholder
    SDL_Rect icon_rect = { bounds_.x + 8, bounds_.y + 8, 40, 40 };
    SDL_SetRenderDrawColor(renderer, 40, 200, 40, 255); // green box for now
    SDL_RenderFillRect(renderer, &icon_rect);
    
    // Draw Name
    Color text_col = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    DrawText(ctx, item_.name, bounds_.x + 56, bounds_.y + 12, text_col);
    
    // Draw quantity text
    if (item_.quantity > 0) {
        std::string qty_str = "x" + std::to_string(item_.quantity);
        Color qty_col = ctx.theme ? ctx.theme->GetTextSecondary() : Color::White;
        DrawText(ctx, qty_str, bounds_.x + 56, bounds_.y + 32, qty_col);
    }
}

bool ItemWidget::HandleInput(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;
        hover_ = (mx >= bounds_.x && mx <= bounds_.x + bounds_.width &&
                  my >= bounds_.y && my <= bounds_.y + bounds_.height);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (hover_) {
            pressed_ = true;
            return true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (pressed_) {
            pressed_ = false;
            if (hover_) {
                if (event.button.button == SDL_BUTTON_LEFT && on_click_) {
                    on_click_();
                } else if (event.button.button == SDL_BUTTON_RIGHT && on_right_click_) {
                    on_right_click_();
                }
                return true;
            }
        }
    }
    return false;
}

void ItemWidget::Update(float dt) {}

} // namespace unboundmp::ui
