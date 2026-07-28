def update_widget_rendering():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_panel_render = '''void Panel::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    if (bg_color_.a > 0) {
        ctx.DrawFilledRect(bounds_, bg_color_);
    }
    
    if (border_thickness_ > 0 && border_color_.a > 0) {
        ctx.DrawOutlinedRect(bounds_, border_color_, border_thickness_);
    }
    
    Container::Render(ctx);
}'''

    new_panel_render = '''void Panel::Render(const RenderContext& ctx) {
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
            ctx.DrawRoundedRect(bounds_, border_color_, corner_radius_);
        } else {
            ctx.DrawOutlinedRect(bounds_, border_color_, border_thickness_);
        }
    }
    
    Container::Render(ctx);
}'''

    content = content.replace(old_panel_render, new_panel_render)

    old_button_render = '''void Button::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color current_color = normal_color_;
    if (!enabled_) current_color = disabled_color_;
    else if (pressed_) current_color = pressed_color_;
    else if (hovered_) current_color = hover_color_;
    
    ctx.DrawFilledRect(bounds_, current_color);
    
    Color text_color{255, 255, 255, 255};
    int tw = MeasureTextWidth(ctx, text_);
    DrawText(ctx, text_, bounds_.x + (bounds_.width - tw) / 2, bounds_.y + (bounds_.height - 14) / 2, text_color);
}'''

    new_button_render = '''void Button::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color current_color = Color::DarkPanel;
    if (!enabled_) current_color = Color::DarkDisabled;
    else if (pressed_) current_color = Color::DarkPressed;
    else if (hovered_) current_color = Color::DarkHover;
    
    ctx.DrawFilledRoundedRect(bounds_, current_color, 4);
    
    if (focused_) {
        ctx.DrawRoundedRect({bounds_.x - 2, bounds_.y - 2, bounds_.width + 4, bounds_.height + 4}, Color::DarkAccent, 6);
    }
    
    Color text_color = Color::DarkText;
    int tw = MeasureTextWidth(ctx, text_);
    DrawText(ctx, text_, bounds_.x + (bounds_.width - tw) / 2, bounds_.y + (bounds_.height - 16) / 2, text_color);
}'''

    content = content.replace(old_button_render, new_button_render)

    old_textbox_render = '''void TextBox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Background
    ctx.DrawFilledRect(bounds_, {30, 30, 30, 255});
    
    // Border
    Color border_color = focused_ ? Color{100, 150, 255, 255} : Color{100, 100, 100, 255};
    ctx.DrawOutlinedRect(bounds_, border_color, 1);
    
    // Text clipping
    ctx.SetClipRect(bounds_);
    
    std::string display_text = password_ ? std::string(text_.length(), '*') : text_;
    
    if (display_text.empty() && !focused_) {
        DrawText(ctx, placeholder_, bounds_.x + 5, bounds_.y + (bounds_.height - 14) / 2, {150, 150, 150, 255});
    } else {
        DrawText(ctx, display_text, bounds_.x + 5, bounds_.y + (bounds_.height - 14) / 2, {255, 255, 255, 255});
        
        // Cursor
        if (focused_ && cursor_visible_) {
            int cx = bounds_.x + 5 + MeasureTextWidth(ctx, display_text.substr(0, cursor_pos_));
            SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, static_cast<Uint8>(255 * ctx.alpha));
            SDL_RenderDrawLine(ctx.renderer, cx, bounds_.y + 5, cx, bounds_.y + bounds_.height - 5);
        }
    }
    
    ctx.ClearClipRect();
}'''

    new_textbox_render = '''void TextBox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    // Background
    ctx.DrawFilledRoundedRect(bounds_, Color::DarkBg, 4);
    
    // Border
    Color border_color = focused_ ? Color::DarkAccent : Color::DarkBorder;
    ctx.DrawRoundedRect(bounds_, border_color, 4);
    
    // Text clipping
    ctx.SetClipRect(bounds_);
    
    std::string display_text = password_ ? std::string(text_.length(), '*') : text_;
    
    if (display_text.empty() && !focused_) {
        DrawText(ctx, placeholder_, bounds_.x + 8, bounds_.y + (bounds_.height - 16) / 2, Color::DarkSubtext);
    } else {
        DrawText(ctx, display_text, bounds_.x + 8, bounds_.y + (bounds_.height - 16) / 2, Color::DarkText);
        
        // Cursor
        if (focused_ && cursor_visible_) {
            int cx = bounds_.x + 8 + MeasureTextWidth(ctx, display_text.substr(0, cursor_pos_));
            SDL_SetRenderDrawColor(ctx.renderer, Color::DarkText.r, Color::DarkText.g, Color::DarkText.b, static_cast<Uint8>(255 * ctx.alpha));
            SDL_RenderDrawLine(ctx.renderer, cx, bounds_.y + 6, cx, bounds_.y + bounds_.height - 6);
        }
    }
    
    ctx.ClearClipRect();
}'''

    content = content.replace(old_textbox_render, new_textbox_render)
    
    old_checkbox_render = '''void Checkbox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Rect box = {bounds_.x, bounds_.y + (bounds_.height - 16) / 2, 16, 16};
    
    // Background
    ctx.DrawFilledRect(box, {40, 40, 40, 255});
    
    // Border
    Color border = hovered_ ? Color{150, 150, 150, 255} : Color{100, 100, 100, 255};
    if (focused_) border = {100, 150, 255, 255};
    ctx.DrawOutlinedRect(box, border, 1);
    
    // Check mark
    if (checked_) {
        ctx.DrawFilledRect({box.x + 4, box.y + 4, 8, 8}, {100, 150, 255, 255});
    }
    
    // Label
    if (!label_.empty()) {
        DrawText(ctx, label_, bounds_.x + 24, bounds_.y + (bounds_.height - 14) / 2, {255, 255, 255, 255});
    }
}'''

    new_checkbox_render = '''void Checkbox::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Rect box = {bounds_.x, bounds_.y + (bounds_.height - 16) / 2, 16, 16};
    
    // Background
    ctx.DrawFilledRoundedRect(box, Color::DarkPanel, 4);
    
    // Border
    Color border = hovered_ ? Color::DarkSubtext : Color::DarkBorder;
    if (focused_) border = Color::DarkAccent;
    ctx.DrawRoundedRect(box, border, 4);
    
    // Check mark
    if (checked_) {
        ctx.DrawFilledRoundedRect({box.x + 3, box.y + 3, 10, 10}, Color::DarkAccent, 2);
    }
    
    // Label
    if (!label_.empty()) {
        DrawText(ctx, label_, bounds_.x + 24, bounds_.y + (bounds_.height - 16) / 2, Color::DarkText);
    }
}'''

    content = content.replace(old_checkbox_render, new_checkbox_render)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

update_widget_rendering()
print("widgets.cpp rendering updated")
