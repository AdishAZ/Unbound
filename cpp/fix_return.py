import glob

def fix_file(path, replacements):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    for old, new in replacements:
        content = content.replace(old, new)
            
    if content != original:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed {path}")

screens_cpp = glob.glob('d:/Unbound/pokemon/cpp/src/ui/screens/*.cpp')
for f in screens_cpp:
    fix_file(f, [
        ('panel_->HandleInput(event);', 'return panel_->HandleInput(event);'),
        ('root_->HandleInput(event);', 'return root_->HandleInput(event);'),
        ('hud_overlay_->HandleInput(event);', 'return hud_overlay_->HandleInput(event);'),
        ('return;', 'return true;')
    ])
