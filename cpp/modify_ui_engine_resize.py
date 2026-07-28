def modify_ui_engine_resize():
    path = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_handle = '''bool UIEngine::HandleInput(const SDL_Event& event) {
    if (!initialized_) return false;
    return screen_manager_->HandleInput(event);
}'''

    new_handle = '''bool UIEngine::HandleInput(const SDL_Event& event) {
    if (!initialized_) return false;
    
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            render_context_.screen_width = event.window.data1;
            render_context_.screen_height = event.window.data2;
            
            if (auto current = screen_manager_->GetCurrentScreen()) {
                current->OnResize(event.window.data1, event.window.data2);
            }
        }
    }
    
    return screen_manager_->HandleInput(event);
}'''

    if 'SDL_WINDOWEVENT_RESIZED' not in content:
        content = content.replace(old_handle, new_handle)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

modify_ui_engine_resize()
print("ui_engine.cpp updated for window resize")
