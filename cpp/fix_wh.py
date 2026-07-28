import re

def fix_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Fix the messed up width/height
    content = re.sub(r'\.width(?:idth)+', '.width', content)
    content = re.sub(r'\.height(?:eight)+', '.height', content)
    
    # Fix regular .w and .h
    content = re.sub(r'\.w\b', '.width', content)
    content = re.sub(r'\.h\b', '.height', content)
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Fixed {path}")

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/popup_menu.cpp')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/context_menu.cpp')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/player_card.cpp')

