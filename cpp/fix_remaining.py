import glob
import re

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    for old, new in replacements:
        if hasattr(old, 'sub'):
            content = old.sub(new, content)
        else:
            content = content.replace(old, new)
        
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

screens_cpp = glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp')
for f in screens_cpp:
    fix_file(f, [
        ('std::make_unique', 'std::make_shared'),
        ('label_->SetLabel', 'label_->SetText'),
        ('label->SetLabel', 'label->SetText'),
        ('engine_->GetScreens()->Push', 'engine_->GetScreens().Push')
    ])

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/player_card.cpp', [
    ('ui/components/avatar_renderer.height', 'ui/components/avatar_renderer.h')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp', [
    ('content_// panel_->Clear();', '// content_panel_->Clear();'),
    ('if (index >= tab_contents_.size()) return true;', 'if (index >= tab_contents_.size()) return;')
])

