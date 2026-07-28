def update_render_context():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_types.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'class ThemeManager;' not in content:
        content = content.replace('class TextRenderer;', 'class TextRenderer;\nclass ThemeManager;')
        content = content.replace('TextRenderer* text_renderer = nullptr;', 'TextRenderer* text_renderer = nullptr;\n    ThemeManager* theme = nullptr;')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('ui_types.h updated')
    else:
        print('ui_types.h already updated')

update_render_context()
