def fix_ui_engine():
    path = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    old_init = '''bool UIEngine::Initialize(SDL_Renderer* renderer, int width, int height) {
    if (initialized_) return true;

    render_context_.renderer = renderer;
    render_context_.screen_width = width;
    render_context_.screen_height = height;'''

    new_init = '''bool UIEngine::Initialize(SDL_Renderer* renderer, int width, int height) {
    if (initialized_) return true;

    render_context_.renderer = renderer;
    render_context_.screen_width = width;
    render_context_.screen_height = height;
    asset_manager_->Initialize(renderer);'''

    if old_init in content:
        content = content.replace(old_init, new_init)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print('Fixed UIEngine renderer bug')
    else:
        print('Failed to find UIEngine::Initialize')

fix_ui_engine()
