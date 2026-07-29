#pragma once

#include <cstdint>
#include <memory>
#include <SDL2/SDL.h>
#include "render/sdl_texture_deleter.h"

namespace unboundmp::render {

// RAII deleters for SDL2 primitives to ensure exception-safe cleanup
struct SDLWindowDeleter {
    void operator()(SDL_Window* window) const noexcept {
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
};

struct SDLRendererDeleter {
    void operator()(SDL_Renderer* renderer) const noexcept {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
};


// Hardware presentation bridge for SDL2.
// Manages window lifecycle, accelerated renderer creation, and framebuffer presentation.
class SDLRenderer {
public:
    SDLRenderer();
    ~SDLRenderer();

    // Prevent copying to ensure exclusive ownership of window and GPU contexts
    SDLRenderer(const SDLRenderer&) = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

    // Allow default move semantics for safe ownership transfer
    SDLRenderer(SDLRenderer&&) noexcept = default;
    SDLRenderer& operator=(SDLRenderer&&) noexcept = default;

    // Initializes window, accelerated renderer, and streaming framebuffer texture
    [[nodiscard]] bool Initialize(int width = 240, int height = 160, int scale = 4);
    
    // Safely tears down hardware contexts and resets state
    void Shutdown();

    // Uploads raw CPU framebuffer pixels to the streaming GPU texture and renders
    void Render(const void* pixels, int pitch);
    
    // Flushes the backbuffer to the display surface (vsync synchronized)
    void Present();

    // Toggles between windowed and borderless desktop fullscreen mode
    void ToggleFullscreen();

    // Polls system window events; returns false if an exit request was received
    [[nodiscard]] bool ProcessEvents();

    // Hardware handle accessors
    [[nodiscard]] SDL_Renderer* GetNativeRenderer() const noexcept { return renderer_.get(); }
    [[nodiscard]] SDL_Window* GetWindow() const noexcept { return window_.get(); }
    [[nodiscard]] bool IsFullscreen() const noexcept { return is_fullscreen_; }

private:
    std::unique_ptr<SDL_Window, SDLWindowDeleter> window_;
    std::unique_ptr<SDL_Renderer, SDLRendererDeleter> renderer_;
    std::unique_ptr<SDL_Texture, SDLTextureDeleter> texture_;
    bool is_fullscreen_ = false;
};

} // namespace unboundmp::render

// Preserve backward compatibility for callers expecting SDLRenderer in the global namespace
using unboundmp::render::SDLRenderer;