def fix_text_renderer():
    path = 'd:/Unbound/pokemon/cpp/src/ui/text_renderer.cpp'
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Fix SDL_Color creation
    old_sdl_color = 'SDL_Color sdl_color = {color.r, color.g, color.b, static_cast<Uint8>(color.a * 255.0f)};'
    new_sdl_color = 'SDL_Color sdl_color = {color.r, color.g, color.b, color.a};'
    content = content.replace(old_sdl_color, new_sdl_color)

    # Fix DrawText alpha mod
    old_alpha_mod = 'SDL_SetTextureAlphaMod(cached->texture, static_cast<Uint8>(color.a * ctx.alpha * 255.0f));'
    new_alpha_mod = 'SDL_SetTextureAlphaMod(cached->texture, static_cast<Uint8>(color.a * ctx.alpha));'
    content = content.replace(old_alpha_mod, new_alpha_mod)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print('Fixed text_renderer.cpp')

fix_text_renderer()
