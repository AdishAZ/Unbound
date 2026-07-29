#pragma once

#include "render/render_layer.h"
#include <SDL2/SDL.h>

namespace unboundmp::render {

// Stub renderer that draws the 240x160 native GBA framebuffer
class MapRenderer : public RenderLayer {
public:
    MapRenderer();
    ~MapRenderer() override;

    void Initialize() override;
    void Shutdown() override;

    void SetFramebuffer(const void* pixels, int pitch);

    void Render(const RenderContext& context) override;

private:
    const void* current_pixels_ = nullptr;
    int current_pitch_ = 0;
    bool is_dirty_ = false;
};

} // namespace unboundmp::render
