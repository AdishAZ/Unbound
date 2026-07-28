def edit_ui_engine_h():
    path = 'd:/Unbound/pokemon/cpp/include/ui/ui_engine.h'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    if '#include "ui/font_manager.h"' not in content:
        content = content.replace('#include "ui/notification_center.h"', '#include "ui/notification_center.h"\n#include "ui/font_manager.h"\n#include "ui/text_renderer.h"')
        
    if 'std::unique_ptr<FontManager> font_manager_;' not in content:
        content = content.replace('NotificationCenter* GetNotificationCenter() { return notification_center_.get(); }', 'NotificationCenter* GetNotificationCenter() { return notification_center_.get(); }\n    FontManager& GetFontManager() { return *font_manager_; }\n    TextRenderer& GetTextRenderer() { return *text_renderer_; }')
        
    if 'std::unique_ptr<FontManager> font_manager_;' not in content:
        content = content.replace('std::unique_ptr<NotificationCenter> notification_center_;', 'std::unique_ptr<NotificationCenter> notification_center_;\n    std::unique_ptr<FontManager> font_manager_;\n    std::unique_ptr<TextRenderer> text_renderer_;')
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

edit_ui_engine_h()
print("ui_engine.h modified")
