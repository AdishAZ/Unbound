#include "render/map_renderer.h"

#include <algorithm>
#include <SDL2/SDL.h>

#include "core/log_manager.h"
#include "render/render_queue.h"
#include "render/resolution.h"
#include "render/viewport.h"

namespace unboundmp::render {

MapRenderer::MapRenderer() : RenderLayer("MapLayer") {}

MapRenderer::~MapRenderer() {
    Shutdown();
}

void MapRenderer::Initialize() {
    // Texture is lazily initialized in Render() once we have a valid native context.
}

void MapRenderer::Shutdown() {
    // Rely on RAII unique_ptr to safely destroy the SDL_Texture
    texture_.reset();
    
    current_pixels_ = nullptr;
    current_pitch_ = 0;
    is_dirty_ = false;
}

void MapRenderer::SetFramebuffer(const void* pixels, int pitch) {
    current_pixels_ = pixels;
    current_pitch_ = pitch;
    is_dirty_ = true;
}

void MapRenderer::Render(const RenderContext& context) {
    if (!context.native_renderer || !context.queue) {
        return;
    }

    if (!current_pixels_) {
        return;
    }

    auto* renderer = static_cast<SDL_Renderer*>(context.native_renderer);

    // Lazy initialization of the streaming texture via RAII
    if (!texture_) {
        texture_.reset(SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            kGbaNativeResolution.width,
            kGbaNativeResolution.height
        ));

        if (!texture_) {
            LOG_ERROR(Client, "MapRenderer: Failed to create hardware texture.");
            return;
        }
        LOG_INFO(Client, "MapRenderer: Hardware texture created successfully.");
    }

    // Sync CPU framebuffer to GPU if dirty
    if (is_dirty_) {
        SDL_UpdateTexture(texture_.get(), nullptr, current_pixels_, current_pitch_);
        is_dirty_ = false;
    }

    // Deduplicate viewport math using the centralized DynamicViewport logic
    DynamicViewport viewport;
    viewport.Resize(Resolution{context.viewport_width, context.viewport_height}, kGbaNativeResolution);
    const ViewportRect& vrect = viewport.Rect();

    // Submit batch command to the render pipeline
    DrawCommand cmd;
    cmd.sort_key.layer = RenderLayerZ::kBackground;
    cmd.texture = texture_.get();
    cmd.src_rect = {0, 0, kGbaNativeResolution.width, kGbaNativeResolution.height};
    cmd.dst_rect = {vrect.x, vrect.y, vrect.width, vrect.height};

    context.queue->Enqueue(cmd);
}

} // namespace unboundmp::render