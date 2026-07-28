def update_login_screen_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/screens/login_screen.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'std::shared_ptr<AnchorLayout> root_layout_;' not in content:
        content = content.replace('std::shared_ptr<Panel> panel_;', 'std::shared_ptr<AnchorLayout> root_layout_;\n    std::shared_ptr<Panel> panel_;')

        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

update_login_screen_h()
print("login_screen.h updated")
