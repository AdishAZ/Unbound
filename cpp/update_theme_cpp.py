def update_theme_cpp():
    path = 'd:/Unbound/pokemon/cpp/src/ui/theme.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    old_default = '''void ThemeManager::SetDefaultTheme() {
    colors_["text"] = {255, 255, 255, 255};
    colors_["background"] = {30, 30, 30, 255};
    colors_["panel"] = {50, 50, 50, 255};
    colors_["primary"] = {100, 150, 255, 255};
    colors_["secondary"] = {150, 150, 150, 255};
    
    ints_["spacing"] = 5;
    ints_["padding"] = 10;
    
    floats_["scale"] = 1.0f;
    
    strings_["font_default"] = "assets/fonts/default.ttf";
}'''

    new_default = '''void ThemeManager::SetDefaultTheme() {
    colors_["text"] = Color::DarkText;
    colors_["text.sub"] = Color::DarkSubtext;
    colors_["background"] = Color::DarkBg;
    colors_["panel"] = Color::DarkPanel;
    colors_["primary"] = Color::DarkAccent;
    colors_["border"] = Color::DarkBorder;
    colors_["hover"] = Color::DarkHover;
    colors_["pressed"] = Color::DarkPressed;
    colors_["disabled"] = Color::DarkDisabled;
    
    ints_["spacing"] = 10;
    ints_["padding"] = 15;
    ints_["corner_radius"] = 6;
    ints_["border_thickness"] = 1;
    ints_["font_size"] = 16;
    ints_["font_size_large"] = 24;
    ints_["font_size_small"] = 12;
    
    floats_["scale"] = 1.0f;
    
    strings_["font_default"] = "assets/fonts/default.ttf";
}'''

    if 'ints_["corner_radius"]' not in content:
        content = content.replace(old_default, new_default)

        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

update_theme_cpp()
print("theme.cpp updated")
