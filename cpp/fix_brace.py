import re

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    for old, new in replacements:
        if hasattr(old, 'sub'):
            content = old.sub(new, content)
        else:
            content = content.replace(old, new)
        
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Fixed {path}")

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/popup_menu.cpp', [
    ('bounds_.y = std::max<int>(0, std::min<int>(bounds_.y, win_h - bounds_.height));\n}', 'bounds_.y = std::max<int>(0, std::min<int>(bounds_.y, win_h - bounds_.height));')
])

