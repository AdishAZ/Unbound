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

# Fix player_card.cpp Render signature
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/player_card.cpp', [
    ('void PlayerCard::Render(RenderContext& ctx)', 'void PlayerCard::Render(const RenderContext& ctx)')
])

# Fix game_screen.cpp ScreenManager unique_ptr
fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/game_screen.cpp', [
    ('std::make_shared<SettingsScreen>', 'std::make_unique<SettingsScreen>')
])

# Fix all screen headers: unique_ptr -> shared_ptr for Widgets
screens_h = glob.glob('d:/Unbound/pokemon/cpp/include/ui/screens/*.h')
for f in screens_h:
    fix_file(f, [
        (re.compile(r'std::unique_ptr<(Panel|Label|TextBox|Checkbox|Button|ProgressBar|Container|Slider|HUDWidget|FPSWidget|PingWidget|Widget)>'), r'std::shared_ptr<\1>')
    ])

# Fix SetLabel -> SetText in cpp files
def fix_setlabel(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    content = content.replace('->SetLabel', '->SetText')
    content = re.sub(r'([a-zA-Z0-9_]*checkbox[a-zA-Z0-9_]*)->SetText', r'\1->SetLabel', content)
    
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

for f in glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp'):
    fix_setlabel(f)

