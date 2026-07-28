#include "render/map_renderer.h"
#include "render/render_queue.h"
#include "core/log_manager.h"

namespace unboundmp::render {

MapRenderer::MapRenderer() : RenderLayer("MapLayer") {}

MapRenderer::~MapRenderer() {
    Shutdown();
}

void MapRenderer::Initialize() {
    // Texture creation will happen on first render or when context is available.
}

void MapRenderer::Shutdown() {
}

void MapRenderer::SetFramebuffer(const void* pixels, int pitch) {
    current_pixels_ = pixels;
    current_pitch_ = pitch;
    is_dirty_ = true;
}

void MapRenderer::Render(const RenderContext& context) {
    if (!context.native_renderer) {
        LOG_ERROR(Client, "MapRenderer: Missing native_renderer");
        return;
    }
    if (!current_pixels_) {
        LOG_INFO(Client, "MapRenderer: No current_pixels_ to render");
        return;
    }
    
    SDL_Renderer* renderer = static_cast<SDL_Renderer*>(context.native_renderer);
    
    // Create texture if it doesn't exist
    // Assuming 240x160 native resolution
    static SDL_Texture* texture = nullptr;
    if (!texture) {
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            240, 160
        );
        LOG_INFO(Client, "MapRenderer: Created Texture (ptr: {})", (void*)texture);
    }
    
    if (is_dirty_) {
        LOG_INFO(Client, "MapRenderer: Updating Texture");
        SDL_UpdateTexture(texture, nullptr, current_pixels_, current_pitch_);
        is_dirty_ = false;
    }
    
    // Draw centered in the viewport
    SDL_Rect dst_rect;
    dst_rect.w = 240;
    dst_rect.h = 160;
    
    // Scale up as much as possible while maintaining 3:2 aspect ratio
    float scale_x = static_cast<float>(context.viewport_width) / 240.0f;
    float scale_y = static_cast<float>(context.viewport_height) / 160.0f;
    float scale = std::min(scale_x, scale_y);
    
    dst_rect.w = static_cast<int>(240.0f * scale);
    dst_rect.h = static_cast<int>(160.0f * scale);
    dst_rect.x = (context.viewport_width - dst_rect.w) / 2;
    dst_rect.y = (context.viewport_height - dst_rect.h) / 2;
    
    // Enqueue the draw command instead of directly rendering
    if (context.queue) {
        DrawCommand cmd;
        cmd.sort_key.layer = RenderLayerZ::kBackground;
        cmd.texture = texture;
        cmd.src_rect = {0, 0, 240, 160};
        cmd.dst_rect = dst_rect;
        context.queue->Enqueue(cmd);
    }
}

} // namespace unboundmp::render
