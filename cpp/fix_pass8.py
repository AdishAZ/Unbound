def fix_ui_engine():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_engine.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if 'std::unique_ptr<NotificationCenter> notification_center_;' not in content:
        content = content.replace('std::unique_ptr<AnimationManager> animation_manager_;', 'std::unique_ptr<AnimationManager> animation_manager_;\n    std::unique_ptr<NotificationCenter> notification_center_;')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

def fix_login_screen():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = content.replace('->SetPasswordMode(', '->SetPassword(')
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

def fix_widgets():
    path = 'd:/Unbound/pokemon/cpp/src/ui/widgets.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    old_logic = '''    if (alignment_ == Alignment::Center) {
        tx += (bounds_.width - tw) / 2;
        ty += (bounds_.height - th) / 2;
    } else if (alignment_ == Alignment::TopRight || alignment_ == Alignment::CenterRight || alignment_ == Alignment::BottomRight) {
        tx += bounds_.width - tw;
    }
    
    if (alignment_ == Alignment::CenterLeft || alignment_ == Alignment::Center || alignment_ == Alignment::CenterRight) {
        ty += (bounds_.height - th) / 2;
    } else if (alignment_ == Alignment::BottomLeft || alignment_ == Alignment::BottomCenter || alignment_ == Alignment::BottomRight) {
        ty += bounds_.height - th;
    }'''
    
    new_logic = '''    if (alignment_ == Alignment::Center) {
        tx += (bounds_.width - tw) / 2;
        ty += (bounds_.height - th) / 2;
    } else if (alignment_ == Alignment::Right) {
        tx += bounds_.width - tw;
        ty += (bounds_.height - th) / 2;
    } else if (alignment_ == Alignment::Left) {
        ty += (bounds_.height - th) / 2;
    }'''
    
    content = content.replace(old_logic, new_logic)
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_ui_engine()
fix_login_screen()
fix_widgets()
