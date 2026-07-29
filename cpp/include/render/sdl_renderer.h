#pragma once

#include <SDL2/SDL.h>
#include <cstdint>

class SDLRenderer {
public:
    SDLRenderer();
    ~SDLRenderer();

    bool Initialize(int width = 240, int height = 160, int scale = 4);
    void Shutdown();

    void Render(const void* pixels, int pitch);
    void Present();
    SDL_Renderer* GetNativeRenderer() const { return renderer_; }
    SDL_Window* GetWindow() const { return window_; }
    bool ProcessEvents();
    void ToggleFullscreen();

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    bool is_fullscreen_ = false;
};