def update_rendering():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Need to include theme.h if not there
    if '#include "ui/theme.h"' not in content:
        content = content.replace('#include "ui/widgets.h"', '#include "ui/widgets.h"\n#include "ui/theme.h"')

    old_panel_render = '''void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    if (bg_color_.a > 0) {
        if (corner_radius_ > 0) {
            ctx.DrawFilledRoundedRect(bounds_, bg_color_, corner_radius_);
        } else {
            ctx.DrawFilledRect(bounds_, bg_color_);
        }
    }
    
    if (border_thickness_ > 0 && border_color_.a > 0) {
        if (corner_radius_ > 0) {
            // Not implemented rounded outlines in ctx yet, draw normal rect
            ctx.DrawOutlinedRect(bounds_, border_color_, border_thickness_);
        } else {
            ctx.DrawOutlinedRect(bounds_, border_color_, border_thickness_);
        }
    }
    
    Container::Render(ctx);
}'''

    new_panel_render = '''void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color bg = (bg_color_.a > 0) ? bg_color_ : (ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel);
    Color border = (border_thickness_ > 0 && border_color_.a > 0) ? border_color_ : (ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder);
    int radius = (corner_radius_ > 0) ? corner_radius_ : (ctx.theme ? ctx.theme->GetCornerRadius() : 4);
    int thick = (border_thickness_ > 0) ? border_thickness_ : (ctx.theme ? ctx.theme->GetBorderThickness() : 1);
    
    ctx.DrawFilledRoundedRect(bounds_, bg, radius);
    if (thick > 0 && border.a > 0) {
        ctx.DrawOutlinedRect(bounds_, border, thick);
    }
    
    Container::Render(ctx);
}'''

    old_window_render = '''void Window::Render(const RenderContext& ctx) {
    if (!visible_) return;
    Panel::Render(ctx);
    
    // Draw title bar
    SDL_SetRenderDrawColor(ctx.renderer, 40, 40, 40, static_cast<Uint8>(255 * ctx.alpha));
    SDL_Rect title_rect = {bounds_.x, bounds_.y, bounds_.width, title_bar_height_};
    SDL_RenderFillRect(ctx.renderer, &title_rect);
    
    Color text_color{255, 255, 255, 255};
    DrawText(ctx, title_, bounds_.x + 10, bounds_.y + (title_bar_height_ - 14) / 2, text_color);
    
    if (closeable_) {
        SDL_SetRenderDrawColor(ctx.renderer, 200, 50, 50, static_cast<Uint8>(255 * ctx.alpha));
        SDL_Rect close_rect = {bounds_.x + bounds_.width - 30, bounds_.y + 4, 20, 20};
        SDL_RenderFillRect(ctx.renderer, &close_rect);
        DrawText(ctx, "X", bounds_.x + bounds_.width - 24, bounds_.y + 7, text_color);
    }
}'''

    new_window_render = '''void Window::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Drop shadow (simple implementation)
    ctx.DrawFilledRoundedRect({bounds_.x + 4, bounds_.y + 4, bounds_.width, bounds_.height}, {0, 0, 0, 100}, 8);
    
    Panel::Render(ctx); // Renders background and borders using Theme
    
    Color title_bg = ctx.theme ? ctx.theme->GetBackground() : Color::FromHex(0x1e2227);
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    
    // Draw title bar
    SDL_Rect title_rect = {bounds_.x, bounds_.y, bounds_.width, title_bar_height_};
    ctx.DrawFilledRoundedRect(title_rect, title_bg, 4); 
    // Fill the bottom half of the rounded rect to make it flat on bottom
    ctx.DrawFilledRect({bounds_.x, bounds_.y + 4, bounds_.width, title_bar_height_ - 4}, title_bg);
    
    // Title text
    DrawText(ctx, title_, bounds_.x + 10, bounds_.y + (title_bar_height_ - 14) / 2, text_color);
    
    if (closeable_) {
        Color close_bg = ctx.theme ? ctx.theme->GetError() : Color::FromHex(0xef4444);
        ctx.DrawFilledRoundedRect({bounds_.x + bounds_.width - 30, bounds_.y + 4, 20, 20}, close_bg, 4);
        DrawText(ctx, "X", bounds_.x + bounds_.width - 24, bounds_.y + 7, text_color);
    }
}'''

    if old_panel_render in content:
        content = content.replace(old_panel_render, new_panel_render)
    if old_window_render in content:
        content = content.replace(old_window_render, new_window_render)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Panel and Window Render updated')

update_rendering()
