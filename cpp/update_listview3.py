def update_listview3():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    start_str = 'void ListView::Render(const RenderContext& ctx) {'
    
    start_idx = content.find(start_str)
    if start_idx == -1: return
    
    # find the end of the function manually
    end_idx = content.find('bool ListView::HandleInput', start_idx)
    if end_idx == -1: return
    
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
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (mx >= bounds_.x && mx <= bounds_.x + bounds_.width && my >= item_y && my <= item_y + item_height_) {
                Color hover_bg = ctx.theme ? ctx.theme->GetSurfaceHover() : Color::FromHex(0x3a404c);
                ctx.DrawFilledRect({bounds_.x, item_y, bounds_.width, item_height_}, hover_bg);
            }
        }
        
        DrawText(ctx, items_[i], bounds_.x + 10, item_y + (item_height_ - 14) / 2, text_color);
    }
    
    ctx.ClearClipRect();
    
    int total_height = static_cast<int>(items_.size()) * item_height_;
    if (total_height > bounds_.height) {
        Color sb_bg = ctx.theme ? ctx.theme->GetSecondary() : Color::FromHex(0x4b5363);
        float scroll_ratio = static_cast<float>(bounds_.height) / total_height;
        int sb_height = std::max(20, static_cast<int>(bounds_.height * scroll_ratio));
        float pos_ratio = static_cast<float>(scroll_y_) / (total_height - bounds_.height);
        int sb_y = bounds_.y + static_cast<int>(pos_ratio * (bounds_.height - sb_height));
        ctx.DrawFilledRoundedRect({bounds_.x + bounds_.width - 10, sb_y, 8, sb_height}, sb_bg, 4);
    }
}

'''
    content = content[:start_idx] + new_listview_render + content[end_idx:]
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('ListView Render updated successfully.')

update_listview3()
