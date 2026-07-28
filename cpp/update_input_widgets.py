def update_input_widgets():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # TextBox Render
    old_textbox = '''    ctx.DrawFilledRoundedRect(bounds_, Color::DarkPanel, 4);
    if (focused_) {
        ctx.DrawOutlinedRect(bounds_, Color::DarkAccent, 2);
    } else {
        ctx.DrawOutlinedRect(bounds_, Color::DarkBorder, 1);
    }'''
    
    new_textbox = '''    Color bg = ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel;
    Color border = focused_ ? (ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent) : (ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder);
    int thick = focused_ ? 2 : 1;
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    
    ctx.DrawFilledRoundedRect(bounds_, bg, radius);
    ctx.DrawOutlinedRect(bounds_, border, thick);'''

    if old_textbox in content:
        content = content.replace(old_textbox, new_textbox)

    # Checkbox Render
    old_checkbox = '''    Rect box_rect = {bounds_.x, bounds_.y + (bounds_.height - 16) / 2, 16, 16};
    ctx.DrawFilledRoundedRect(box_rect, Color::DarkPanel, 4);
    ctx.DrawOutlinedRect(box_rect, focused_ ? Color::DarkAccent : Color::DarkBorder, 1);
    
    if (checked_) {
        Rect check_inner = {box_rect.x + 4, box_rect.y + 4, 8, 8};
        ctx.DrawFilledRoundedRect(check_inner, Color::DarkAccent, 2);
    }
    
    Color text_color = Color::DarkText;
    DrawText(ctx, label_, box_rect.x + 24, bounds_.y + (bounds_.height - 16) / 2, text_color);'''
    
    new_checkbox = '''    Rect box_rect = {bounds_.x, bounds_.y + (bounds_.height - 16) / 2, 16, 16};
    
    Color bg = ctx.theme ? ctx.theme->GetSurface() : Color::DarkPanel;
    Color border = focused_ ? (ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent) : (ctx.theme ? ctx.theme->GetBorder() : Color::DarkBorder);
    Color check_color = ctx.theme ? ctx.theme->GetPrimary() : Color::DarkAccent;
    Color text_color = ctx.theme ? ctx.theme->GetTextPrimary() : Color::DarkText;
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    
    ctx.DrawFilledRoundedRect(box_rect, bg, radius);
    ctx.DrawOutlinedRect(box_rect, border, 1);
    
    if (checked_) {
        Rect check_inner = {box_rect.x + 4, box_rect.y + 4, 8, 8};
        ctx.DrawFilledRoundedRect(check_inner, check_color, radius > 2 ? radius - 2 : 0);
    }
    
    DrawText(ctx, label_, box_rect.x + 24, bounds_.y + (bounds_.height - 14) / 2, text_color);'''

    if old_checkbox in content:
        content = content.replace(old_checkbox, new_checkbox)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('TextBox and Checkbox updated')

update_input_widgets()
