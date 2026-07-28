def edit_ui_engine_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add allocations
    if 'font_manager_ = std::make_unique<FontManager>();' not in content:
        content = content.replace('notification_center_ = std::make_unique<NotificationCenter>();', 'notification_center_ = std::make_unique<NotificationCenter>();\n    font_manager_ = std::make_unique<FontManager>();\n    text_renderer_ = std::make_unique<TextRenderer>(font_manager_.get());')

    # Add initialize
    old_init = '    initialized_ = true;\n    return true;'
    new_init = '''    if (!font_manager_->Initialize("assets/fonts/default.ttf")) {
        return false;
    }
    
    render_context_.font_manager = font_manager_.get();
    render_context_.text_renderer = text_renderer_.get();

    initialized_ = true;
    return true;'''
    content = content.replace(old_init, new_init)
    
    # Add shutdown
    old_shutdown = '    animation_manager_->Clear();'
    new_shutdown = '    animation_manager_->Clear();\n    text_renderer_->ClearCache();\n    font_manager_->Shutdown();'
    content = content.replace(old_shutdown, new_shutdown)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_ui_engine_cpp()
print("ui_engine.cpp modified")
