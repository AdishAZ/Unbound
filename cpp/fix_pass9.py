def fix_replace():
    files = [
        'd:/Unbound/pokemon/cpp/src/ui/screens/login_screen.cpp',
        'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp',
        'd:/Unbound/pokemon/cpp/src/ui/screens/loading_screen.cpp'
    ]
    for path in files:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('Replace(std::make_shared<', 'Replace(std::make_unique<')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

def fix_avatar_renderer():
    paths = [
        'd:/Unbound/pokemon/cpp/include/ui/components/avatar_renderer.h',
        'd:/Unbound/pokemon/cpp/src/ui/components/avatar_renderer.cpp'
    ]
    for path in paths:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('void Render(RenderContext& ctx, Rect bounds, uint64_t id)', 'void Render(const RenderContext& ctx, Rect bounds, uint64_t id)')
        content = content.replace('void AvatarRenderer::Render(RenderContext& ctx, Rect bounds, uint64_t id)', 'void AvatarRenderer::Render(const RenderContext& ctx, Rect bounds, uint64_t id)')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

fix_replace()
fix_avatar_renderer()
