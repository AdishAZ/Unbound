import re

def fix_ui_engine():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_engine.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if '#include "ui/notification_center.h"' not in content:
        content = content.replace('#include "ui/localization.h"', '#include "ui/localization.h"\n#include "ui/notification_center.h"')
    
    if 'GetNotificationCenter()' not in content:
        content = content.replace('AnimationManager& GetAnimations() { return *animation_manager_; }', 'AnimationManager& GetAnimations() { return *animation_manager_; }\n    NotificationCenter* GetNotificationCenter() { return notification_center_.get(); }')
        
    if 'notification_center_' not in content:
        content = content.replace('std::unique_ptr<AnimationManager> animation_manager_;', 'std::unique_ptr<AnimationManager> animation_manager_;\n    std::unique_ptr<NotificationCenter> notification_center_;')
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
        
    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/ui_engine.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()
        
    if 'notification_center_ = std::make_unique<NotificationCenter>();' not in content_cpp:
        content_cpp = content_cpp.replace('animation_manager_ = std::make_unique<AnimationManager>();', 'animation_manager_ = std::make_unique<AnimationManager>();\n    notification_center_ = std::make_unique<NotificationCenter>();')
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)

def fix_char_select():
    path = 'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = content.replace('character_list_->SetOnSelectionChanged', 'character_list_->OnSelectionChanged')
    content = content.replace('character_list_->Clear()', 'character_list_->ClearItems()')
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

fix_ui_engine()
fix_char_select()
