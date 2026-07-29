#pragma once

#include <memory>
#include <SDL2/SDL.h>

#include "render/render_layer.h"
#include "render/sdl_texture_deleter.h"

namespace unboundmp::render {

// RAII deleter for SDL_Texture to guarantee safe destruction


// Renders the 240x160 native GBA framebuffer to the viewport.
// Handles dynamic texture streaming, dirty-state tracking, and hardware scaling.
class MapRenderer : public RenderLayer {
public:
    MapRenderer();
    ~MapRenderer() override;

    // Prevent copying to ensure exclusive ownership of the hardware texture
    MapRenderer(const MapRenderer&) = delete;
    MapRenderer& operator=(const MapRenderer&) = delete;

    // Allow default move semantics for safe transfer
    MapRenderer(MapRenderer&&) noexcept = default;
    MapRenderer& operator=(MapRenderer&&) noexcept = default;

    // Prepares the renderer (lazy texture initialization happens in Render)
    void Initialize() override;
    
    // Safely destroys GPU resources and resets internal state
    void Shutdown() override;

    // Submits a new raw CPU pixel buffer for GPU upload on the next frame
    void SetFramebuffer(const void* pixels, int pitch);

    // Submits the map draw command to the centralized render queue
    void Render(const RenderContext& context) override;

private:
    std::unique_ptr<SDL_Texture, SDLTextureDeleter> texture_;
    const void* current_pixels_ = nullptr;
    int current_pitch_ = 0;
    bool is_dirty_ = false;
};

} // namespace unboundmp::render