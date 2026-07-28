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

screens_h = glob.glob('d:/Unbound/pokemon/cpp/include/ui/screens/*.h')
for f in screens_h:
    fix_file(f, [
        ('void Render(RenderContext& ctx)', 'void Render(const RenderContext& ctx)'),
        ('void HandleInput(const SDL_Event& event)', 'bool HandleInput(const SDL_Event& event)')
    ])
