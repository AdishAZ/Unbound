def fix_settings_cpp():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/settings_screen.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'void OnResize(int width, int height)' not in content_h:
        content_h = content_h.replace('void OnResume() override;', 'void OnResume() override;\n    void OnResize(int width, int height) override;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)
            
    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    # Replace panel_ with a local variable since we use root_layout_
    old_panel = '''    panel_ = std::make_shared<Window>("settings_win");
    ((Window*)panel_.get())->SetTitle(L("settings.title"));
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 600, 450});'''
    
    new_panel = '''    auto panel_ = std::make_shared<Window>("settings_win");
    panel_->SetTitle(L("settings.title"));
    panel_->SetAnchor(AnchorPoint::Center);
    panel_->SetBounds({0, 0, 600, 450});'''
    
    if old_panel in content_cpp:
        content_cpp = content_cpp.replace(old_panel, new_panel)
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)
    print('Fixed settings screen compilation errors')

fix_settings_cpp()
