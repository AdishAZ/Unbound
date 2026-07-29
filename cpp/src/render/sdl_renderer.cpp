#include "render/sdl_renderer.h"

SDLRenderer::SDLRenderer()
    : window_(nullptr),
      renderer_(nullptr),
      texture_(nullptr)
{
}

SDLRenderer::~SDLRenderer()
{
    Shutdown();
}

bool SDLRenderer::Initialize(int width, int height, int scale)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    window_ = SDL_CreateWindow(
        "Pokemon Unbound",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width * scale,
        height * scale,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window_)
        return false;

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer_)
        return false;
        
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    // Removed SDL_RenderSetLogicalSize to render at exact physical pixel size!

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

    return texture_ != nullptr;
}

void SDLRenderer::Shutdown()
{
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);

    texture_ = nullptr;
    renderer_ = nullptr;
    window_ = nullptr;

    SDL_Quit();
}

void SDLRenderer::ToggleFullscreen()
{
    if (!window_) return;
    
    is_fullscreen_ = !is_fullscreen_;
    if (is_fullscreen_) {
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(window_, 0);
    }
}

bool SDLRenderer::ProcessEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return false;
    }

    return true;
}

void SDLRenderer::Render(const void* pixels, int pitch)
{
    SDL_UpdateTexture(texture_, nullptr, pixels, pitch);

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
}

void SDLRenderer::Present()
{
    SDL_RenderPresent(renderer_);
}