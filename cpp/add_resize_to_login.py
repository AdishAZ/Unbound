def add_resize_to_login():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/login_screen.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'void OnResize(int width, int height) override;' not in content_h:
        content_h = content_h.replace('void OnResume() override;', 'void OnResume() override;\n    void OnResize(int width, int height) override;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if 'void LoginScreen::OnResize' not in content_cpp:
        content_cpp = content_cpp.replace('void LoginScreen::OnResume() {}', 'void LoginScreen::OnResume() {}\n\nvoid LoginScreen::OnResize(int width, int height) {\n    if (root_layout_) {\n        root_layout_->SetBounds({0, 0, width, height});\n        root_layout_->InvalidateLayout();\n    }\n}')
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)

add_resize_to_login()
print("LoginScreen updated with OnResize")
