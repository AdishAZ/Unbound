def update_ui_engine_init():
    path = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_init = '''    render_context_.font_manager = font_manager_.get();
    render_context_.text_renderer = text_renderer_.get();'''
    
    new_init = '''    render_context_.font_manager = font_manager_.get();
    render_context_.text_renderer = text_renderer_.get();
    render_context_.theme = theme_manager_.get();'''
    
    if old_init in content:
        content = content.replace(old_init, new_init)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('ui_engine.cpp updated')
    else:
        print('Could not find old_init in ui_engine.cpp')

update_ui_engine_init()
