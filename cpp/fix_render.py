import re

def fix_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = content.replace('void Render(RenderContext& ctx)', 'void Render(const RenderContext& ctx)')
    
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Fixed {path}")

fix_file('d:/Unbound/pokemon/cpp/include/ui/components/popup_menu.h')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/popup_menu.cpp')
fix_file('d:/Unbound/pokemon/cpp/include/ui/components/context_menu.h')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/context_menu.cpp')
fix_file('d:/Unbound/pokemon/cpp/include/ui/components/player_card.h')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/player_card.cpp')
fix_file('d:/Unbound/pokemon/cpp/include/ui/components/avatar_renderer.h')
fix_file('d:/Unbound/pokemon/cpp/src/ui/components/avatar_renderer.cpp')

