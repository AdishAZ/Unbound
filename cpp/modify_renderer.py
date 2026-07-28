def modify_renderer():
    # sdl_renderer.h
    h_path = 'd:/Unbound/pokemon/cpp/include/render/sdl_renderer.h'
    with open(h_path, 'r', encoding='utf-8') as f:
        h_content = f.read()
    
    h_content = h_content.replace('void Render(const void* pixels, int pitch);', 'void Render(const void* pixels, int pitch);\n    void Present();\n    SDL_Renderer* GetNativeRenderer() const { return renderer_; }')
    
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write(h_content)
        
    # sdl_renderer.cpp
    cpp_path = 'd:/Unbound/pokemon/cpp/src/render/sdl_renderer.cpp'
    with open(cpp_path, 'r', encoding='utf-8') as f:
        cpp_content = f.read()
        
    old_render = '''void SDLRenderer::Render(const void* pixels, int pitch)
{
    SDL_UpdateTexture(texture_, nullptr, pixels, pitch);

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}'''
    
    new_render = '''void SDLRenderer::Render(const void* pixels, int pitch)
{
    SDL_UpdateTexture(texture_, nullptr, pixels, pitch);

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
}

void SDLRenderer::Present()
{
    SDL_RenderPresent(renderer_);
}'''

    cpp_content = cpp_content.replace(old_render, new_render)
    
    with open(cpp_path, 'w', encoding='utf-8') as f:
        f.write(cpp_content)

modify_renderer()
print("Renderer modified")
