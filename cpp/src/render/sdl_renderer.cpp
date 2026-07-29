#include "render/sdl_renderer.h"

#include <algorithm>
#include <SDL2/SDL.h>

#include "core/log_manager.h"
#include "render/resolution.h"

namespace unboundmp::render {

SDLRenderer::SDLRenderer() = default;

SDLRenderer::~SDLRenderer() {
    Shutdown();
}

bool SDLRenderer::Initialize(int width, int height, int scale) {
    if (width <= 0 || height <= 0 || scale <= 0) {
        LOG_ERROR(Client, "SDLRenderer: Invalid initialization dimensions ({}x{}, scale: {})", width, height, scale);
        return false;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        LOG_ERROR(Client, "SDLRenderer: Failed to initialize SDL video subsystem: {}", SDL_GetError());
        return false;
    }

    window_.reset(SDL_CreateWindow(
        "Pokemon Unbound",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width * scale,
        height * scale,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    ));

    if (!window_) {
        LOG_ERROR(Client, "SDLRenderer: Failed to create window: {}", SDL_GetError());
        Shutdown();
        return false;
    }

    renderer_.reset(SDL_CreateRenderer(
        window_.get(),
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    ));

    if (!renderer_) {
        LOG_ERROR(Client, "SDLRenderer: Failed to create hardware renderer: {}", SDL_GetError());
        Shutdown();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer_.get(), SDL_BLENDMODE_BLEND);

    texture_.reset(SDL_CreateTexture(
        renderer_.get(),
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    ));

    if (!texture_) {
        LOG_ERROR(Client, "SDLRenderer: Failed to create streaming framebuffer texture: {}", SDL_GetError());
        Shutdown();
        return false;
    }

    is_fullscreen_ = false;
    LOG_INFO(Client, "SDLRenderer: Initialized successfully ({}x{} @ {}x scale)", width, height, scale);
    return true;
}

void SDLRenderer::Shutdown() {
    texture_.reset();
    renderer_.reset();
    window_.reset();

    if (SDL_WasInit(SDL_INIT_VIDEO) != 0) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    is_fullscreen_ = false;
}

void SDLRenderer::ToggleFullscreen() {
    if (!window_) {
        return;
    }

    is_fullscreen_ = !is_fullscreen_;
    const Uint32 fullscreen_flag = is_fullscreen_ ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

    if (SDL_SetWindowFullscreen(window_.get(), fullscreen_flag) != 0) {
        LOG_ERROR(Client, "SDLRenderer: Failed to toggle fullscreen mode: {}", SDL_GetError());
        is_fullscreen_ = !is_fullscreen_; // Revert state on failure
    }
}

bool SDLRenderer::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

void SDLRenderer::Render(const void* pixels, int pitch) {
    if (!renderer_ || !texture_ || !pixels || pitch <= 0) {
        return;
    }

    if (SDL_UpdateTexture(texture_.get(), nullptr, pixels, pitch) != 0) {
        LOG_ERROR(Client, "SDLRenderer: Failed to update streaming texture: {}", SDL_GetError());
        return;
    }

    SDL_RenderClear(renderer_.get());
    SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
}

void SDLRenderer::Present() {
    if (renderer_) {
        SDL_RenderPresent(renderer_.get());
    }
}

} // namespace unboundmp::render