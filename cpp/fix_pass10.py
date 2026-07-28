import re

def fix_avatar_renderer():
    paths = [
        'd:/Unbound/pokemon/cpp/include/ui/components/avatar_renderer.h',
        'd:/Unbound/pokemon/cpp/src/ui/components/avatar_renderer.cpp'
    ]
    for path in paths:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
        content = re.sub(r'Render\(RenderContext&\s+ctx,\s*Rect\s+bounds,\s*uint64_t\s+(\w+)\)', r'Render(const RenderContext& ctx, Rect bounds, uint64_t \1)', content)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

fix_avatar_renderer()
