import os
import glob
import re

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    for old, new in replacements:
        if hasattr(old, 'sub'):
            content = old.sub(new, content)
        elif callable(old):
            content = old(content)
        else:
            content = content.replace(old, new)
            
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

screens_h = glob.glob('d:/Unbound/pokemon/cpp/include/ui/screens/*.h')
screens_cpp = glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp')

for f in screens_cpp:
    fix_file(f, [
        (re.compile(r'void\s+(\w+Screen)::HandleInput\s*\('), r'bool \1::HandleInput('),
        ('void Render(RenderContext& ctx)', 'void Render(const RenderContext& ctx)'),
        (re.compile(r'void\s+(\w+Screen)::Render\s*\(\s*RenderContext&\s*ctx\s*\)'), r'void \1::Render(const RenderContext& ctx)'),
        ('GetScreens()->Replace', 'GetScreens().Replace'),
        ('std::make_shared', 'std::make_unique'),
        ('SetOnClick', 'OnClick'),
        ('->SetText', '->SetLabel'),
        ('panel_->Clear()', '// panel_->Clear()')
    ])
    
fix_file('d:/Unbound/pokemon/cpp/src/ui/screens/settings_screen.cpp', [
    ('checkbox->SetText', 'checkbox->SetLabel'),
    ('->SetText(', '->SetLabel(')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/components/popup_menu.cpp', [
    ('.w =', '.width ='),
    ('.h =', '.height ='),
    ('.w -', '.width -'),
    ('.h -', '.height -'),
    ('.w;', '.width;'),
    ('.h;', '.height;'),
    ('bounds_.w', 'bounds_.width'),
    ('bounds_.h', 'bounds_.height'),
    ('bounds.w', 'bounds.width'),
    ('bounds.h', 'bounds.height'),
    ('anchor_bounds.w', 'anchor_bounds.width'),
    ('anchor_bounds.h', 'anchor_bounds.height')
])

fix_file('d:/Unbound/pokemon/cpp/include/ui/components/popup_menu.h', [
    (re.compile(r'enum class AnchorPoint.*?};', re.DOTALL), ''),
    ('#include "ui/widget.h"', '#include "ui/widget.h"\n#include "ui/ui_types.h"')
])

fix_file('d:/Unbound/pokemon/cpp/src/ui/widgets.cpp', [
    ('static_cast<Uint8>(150 * ctx.alpha)', '150')
])

fix_file('d:/Unbound/pokemon/cpp/include/ui/widgets.h', [
    ('Alignment::TopLeft', 'Alignment::Left')
])

