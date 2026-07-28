def update_listview():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_listview_render = '''void ListView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 20, static_cast<Uint8>(255 * ctx.alpha));
    SDL_Rect bg = {bounds_.x, bounds_.y, bounds_.width, bounds_.height};
    SDL_RenderFillRect(ctx.renderer, &bg);
    
    SDL_Rect prev_clip;
    SDL_RenderGetClipRect(ctx.renderer, &prev_clip);
    SDL_RenderSetClipRect(ctx.renderer, &bg);
    
    for (size_t i = 0; i < items_.size(); ++i) {
        int item_y = bounds_.y + static_cast<int>(i) * item_height_ - scroll_y_;
        if (item_y + item_height_ < bounds_.y || item_y > bounds_.y + bounds_.height) continue;
        
        if (static_cast<int>(i) == selected_index_) {
            SDL_SetRenderDrawColor(ctx.renderer, 60, 100, 160, static_cast<Uint8>(255 * ctx.alpha));
            SDL_Rect sel_bg = {bounds_.x, item_y, bounds_.width, item_height_};
            SDL_RenderFillRect(ctx.renderer, &sel_bg);
        }
        
        Color text_color{255, 255, 255, 255};
        DrawText(ctx, items_[i], bounds_.x + 10, item_y + (item_height_ - 14) / 2, text_color);
    }
    
    SDL_RenderSetClipRect(ctx.renderer, &prev_clip);
}'''

    new_listview_render = '''void ListView::Render(const RenderContext& ctx) {
    if (!visible_) return;
    
    Color bg = ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel;
    Color border = ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder;
    Color sel_color = ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent;
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::White;
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    
    ctx.DrawFilledRoundedRect(bounds_, bg, radius);
    ctx.DrawOutlinedRect(bounds_, border, 1);
    
    ctx.SetClipRect(bounds_);
    
    for (size_t i = 0; i < items_.size(); ++i) {
        int item_y = bounds_.y + static_cast<int>(i) * item_height_ - scroll_y_;
        if (item_y + item_height_ < bounds_.y || item_y > bounds_.y + bounds_.height) continue;
        
        if (static_cast<int>(i) == selected_index_) {
            ctx.DrawFilledRect({bounds_.x, item_y, bounds_.width, item_height_}, sel_color);
        } else {
            // Check hover (simple manual hit test since list items aren't widgets)
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (mx >= bounds_.x && mx <= bounds_.x + bounds_.width && my >= item_y && my <= item_y + item_height_) {
                Color hover_bg = ctx.theme ? ctx.theme->GetSurfaceHover() : Color::DarkHover;
                ctx.DrawFilledRect({bounds_.x, item_y, bounds_.width, item_height_}, hover_bg);
            }
        }
        
        DrawText(ctx, items_[i], bounds_.x + 10, item_y + (item_height_ - 14) / 2, text_color);
    }
    
    ctx.ClearClipRect();
}'''

    if old_listview_render in content:
        content = content.replace(old_listview_render, new_listview_render)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('ListView Render updated')
    else:
        print('ListView Render not found')

update_listview()
