def update_widgets_render():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Update Button
    old_btn = '''    Color current_color = disabled_color_;
    if (enabled_) {
        current_color = pressed_ ? pressed_color_ : (hovered_ ? hover_color_ : normal_color_);
    }
    
    ctx.DrawFilledRoundedRect(bounds_, current_color, 4);'''
    
    new_btn = '''    Color base_color = ctx.theme ? ctx.theme->GetPrimary() : normal_color_;
    Color hover = ctx.theme ? Color::Lerp(base_color, Color::White, 0.2f) : hover_color_;
    Color pressed = ctx.theme ? Color::Lerp(base_color, Color::Black, 0.2f) : pressed_color_;
    Color disabled = ctx.theme ? ctx.theme->GetSecondary() : disabled_color_;
    
    Color current_color = disabled;
    if (enabled_) {
        current_color = pressed_ ? pressed : (hovered_ ? hover : base_color);
    }
    
    int radius = ctx.theme ? ctx.theme->GetCornerRadius() : 4;
    ctx.DrawFilledRoundedRect(bounds_, current_color, radius);'''

    if old_btn in content:
        content = content.replace(old_btn, new_btn)

    # Update Label
    old_label = '''    DrawText(ctx, text_, bounds_.x, bounds_.y, color_);'''
    new_label = '''    Color text_color = (color_.a > 0 && color_.r != 255 && color_.g != 255) ? color_ : (ctx.theme ? ctx.theme->GetTextPrimary() : color_);
    
    int tx = bounds_.x;
    int ty = bounds_.y;
    int tw, th;
    Widget::MeasureText(ctx, text_, tw, th);
    
    if (alignment_ == Alignment::Center) {
        tx = bounds_.x + (bounds_.width - tw) / 2;
        ty = bounds_.y + (bounds_.height - th) / 2;
    }
    
    DrawText(ctx, text_, tx, ty, text_color);'''

    if old_label in content:
        content = content.replace(old_label, new_label)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Button and Label updated')

update_widgets_render()
