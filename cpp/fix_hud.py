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

fix_file('d:/Unbound/pokemon/cpp/include/ui/hud_widgets.h', [
    (re.compile(r'enum class AnchorPoint.*?};', re.DOTALL), '')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/hud_widgets.cpp', [
    ('m_rect.w', 'bounds_.width'),
    ('m_rect.h', 'bounds_.height'),
    ('m_rect.x', 'bounds_.x'),
    ('m_rect.y', 'bounds_.y')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/widget.cpp', [
    ('color.a * ctx.alpha', 'color.a * 1.0f')
])

fix_file('d:/Unbound/pokemon/cpp/include/ui/widget.h', [
    (re.compile(r'(// Helper: draw text as monospace)'), r'public:\n    \1')
])

