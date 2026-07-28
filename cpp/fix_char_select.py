def fix_char_select():
    path_h = 'd:/Unbound/pokemon/cpp/include/ui/screens/character_select_screen.h'
    with open(path_h, 'r', encoding='utf-8') as f:
        content_h = f.read()

    if 'std::shared_ptr<Container> root_layout_;' not in content_h:
        content_h = content_h.replace('private:', 'private:\n    std::shared_ptr<Container> root_layout_;')
        content_h = content_h.replace('void OnResume() override;', 'void OnResume() override;\n    void OnResize(int width, int height) override;')
        with open(path_h, 'w', encoding='utf-8') as f:
            f.write(content_h)

    path_cpp = 'd:/Unbound/pokemon/cpp/src/ui/screens/character_select_screen.cpp'
    with open(path_cpp, 'r', encoding='utf-8') as f:
        content_cpp = f.read()

    if '#include "ui/screens/game_screen.h"' not in content_cpp:
        content_cpp = content_cpp.replace('#include "ui/screens/character_select_screen.h"', '#include "ui/screens/character_select_screen.h"\n#include "ui/screens/game_screen.h"')

    # Fix OnResize, Render, HandleInput, Update which I apparently completely omitted from my rewrite of character_select_screen.cpp
    # Wait, the old file had OnResize, Render, HandleInput, Update, I just injected the new Init at the top!
    # I replaced old_init_start to old_init_end, so the rest of the file should still be there.
    # But wait, did I use root_layout_ in those methods? Let's add them explicitly to be safe.
    
    old_methods = '''void CharacterSelectScreen::OnEnter() {}
void CharacterSelectScreen::OnExit() {}
void CharacterSelectScreen::OnPause() {}
void CharacterSelectScreen::OnResume() {}'''

    new_methods = '''void CharacterSelectScreen::OnEnter() {}
void CharacterSelectScreen::OnExit() {}
void CharacterSelectScreen::OnPause() {}
void CharacterSelectScreen::OnResume() {}

void CharacterSelectScreen::OnResize(int width, int height) {
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, width, height});
        root_layout_->InvalidateLayout();
    }
}

void CharacterSelectScreen::Render(const RenderContext& ctx) {
    SDL_SetRenderDrawColor(ctx.renderer, 30, 30, 40, 255);
    SDL_RenderClear(ctx.renderer);
    if (root_layout_) {
        root_layout_->SetBounds({0, 0, ctx.screen_width, ctx.screen_height});
        root_layout_->Render(ctx);
    }
}

bool CharacterSelectScreen::HandleInput(const SDL_Event& event) {
    if (root_layout_) return root_layout_->HandleInput(event);
    return false;
}

void CharacterSelectScreen::Update(float dt) {
    if (root_layout_) root_layout_->Update(dt);
}
'''
    if old_methods in content_cpp:
        idx = content_cpp.find(old_methods)
        content_cpp = content_cpp[:idx] + new_methods + '\n} // namespace unboundmp::ui\n'
        with open(path_cpp, 'w', encoding='utf-8') as f:
            f.write(content_cpp)

    print('Fixed character select screen')

fix_char_select()
