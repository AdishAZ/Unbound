def make_wm_global():
    path = 'd:/Unbound/pokemon/cpp/include/ui/window_manager.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'static WindowManager* GetInstance();' not in content:
        content = content.replace('class WindowManager {', 'class WindowManager {\npublic:\n    static WindowManager* GetInstance();\n')
        content = content.replace('void SetBounds(const Rect& bounds) { bounds_ = bounds; }', 'void SetBounds(const Rect& bounds) { bounds_ = bounds; }\n    void ShowPopup(std::shared_ptr<Widget> popup, int x, int y);\n    void ClosePopup();')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/window_manager.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'WindowManager* WindowManager::GetInstance()' not in content:
        content = content.replace('WindowManager::WindowManager() {}', 'static WindowManager* g_wm = nullptr;\n\nWindowManager::WindowManager() { g_wm = this; }\n\nWindowManager* WindowManager::GetInstance() { return g_wm; }')
        
        popup_impl = '''
void WindowManager::ShowPopup(std::shared_ptr<Widget> popup, int x, int y) {
    popup->SetBounds({x, y, popup->GetBounds().width, popup->GetBounds().height});
    popups_.clear(); // Only one popup at a time
    popups_.push_back(popup);
}

void WindowManager::ClosePopup() {
    popups_.clear();
}
'''
        content = content.replace('} // namespace unboundmp::ui', popup_impl + '\n} // namespace unboundmp::ui')
        
        # update Render
        old_render = '''    if (modal_window_ && modal_window_->IsVisible()) {
        // Draw dark overlay
        ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {0, 0, 0, 150});
        modal_window_->Render(ctx);
    }'''
        new_render = '''    if (modal_window_ && modal_window_->IsVisible()) {
        // Draw dark overlay
        ctx.DrawFilledRect({0, 0, ctx.screen_width, ctx.screen_height}, {0, 0, 0, 150});
        modal_window_->Render(ctx);
    }
    
    for (auto& p : popups_) {
        if (p->IsVisible()) p->Render(ctx);
    }'''
        content = content.replace(old_render, new_render)
        
        # update Update
        old_update = '''    if (modal_window_) {
        modal_window_->Update(dt);
    }'''
        new_update = '''    if (modal_window_) {
        modal_window_->Update(dt);
    }
    for (auto& p : popups_) p->Update(dt);'''
        content = content.replace(old_update, new_update)
        
        # update HandleInput
        old_input = '''bool WindowManager::HandleInput(const SDL_Event& event) {
    if (modal_window_ && modal_window_->IsVisible()) {'''
        
        new_input = '''bool WindowManager::HandleInput(const SDL_Event& event) {
    if (!popups_.empty()) {
        if (popups_.back()->HandleInput(event)) return true;
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            ClosePopup(); // Click outside popup closes it
            return true;
        }
    }

    if (modal_window_ && modal_window_->IsVisible()) {'''
        content = content.replace(old_input, new_input)
        
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content)
        print('WindowManager updated for global popups')

make_wm_global()
