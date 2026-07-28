def update_settings_header():
    path = 'd:/Unbound/pokemon/cpp/include/ui/screens/settings_screen.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if 'std::shared_ptr<Widget> root_layout_;' not in content:
        content = content.replace('std::shared_ptr<Panel> panel_;', 'std::shared_ptr<Widget> root_layout_;')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
    print('SettingsScreen header updated')

update_settings_header()
