def edit_widgets_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    content = content.replace('MeasureTextWidth(text_)', 'MeasureTextWidth(ctx, text_)')
    content = content.replace('MeasureTextWidth(text)', 'MeasureTextWidth(ctx, text)')
    content = content.replace('MeasureTextWidth(display_text.substr(0, cursor_pos_))', 'MeasureTextWidth(ctx, display_text.substr(0, cursor_pos_))')

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_widgets_cpp()
print("widgets.cpp modified")
