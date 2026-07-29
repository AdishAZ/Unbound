#include "render/remote_player_renderer.h"

#include <algorithm>
#include <iostream>
#include <string>

#include <SDL2/SDL.h>

#include "core/log_manager.h"
#include "gameplay/remote_player_manager.h"
#include "render/camera.h"
#include "render/render_queue.h"
#include "render/resolution.h"
#include "render/viewport.h"

namespace unboundmp::render {

RemotePlayerRenderer::RemotePlayerRenderer(std::shared_ptr<gameplay::RemotePlayerManager> manager)
    : RenderLayer("RemotePlayerRenderer"), manager_(std::move(manager)) {}

RemotePlayerRenderer::~RemotePlayerRenderer() {
    Shutdown();
}

void RemotePlayerRenderer::Initialize() {
    // Pre-loading happens here if a native renderer context is already available,
    // or is safely deferred to LoadSpritesIfNecessary() on the first render frame.
}

void RemotePlayerRenderer::Shutdown() {
    // Rely on RAII unique_ptr deleters to destroy SDL_Texture handles safely
    trainer_sprites_.clear();
}

void RemotePlayerRenderer::LoadSpritesIfNecessary(SDL_Renderer* renderer) {
    if (!renderer || !trainer_sprites_.empty()) {
        return;
    }

    trainer_sprites_.reserve(10);

    for (int i = 0; i < 10; ++i) {
        const std::string path = "assets/characters/trainer_" + std::to_string(i) + ".bmp";
        SDL_Surface* surface = SDL_LoadBMP(path.c_str());

        if (!surface) {
            std::cerr << "[RemotePlayerRenderer] Failed to load BMP: " << path 
                      << " (" << SDL_GetError() << ")\n";
            trainer_sprites_.push_back(nullptr);
            continue;
        }

        // Set magenta (255, 0, 255) as transparent color key
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
        
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!tex) {
            std::cerr << "[RemotePlayerRenderer] Failed to create texture for: " << path 
                      << " (" << SDL_GetError() << ")\n";
        }

        trainer_sprites_.push_back(std::unique_ptr<SDL_Texture, SDLTextureDeleter>(tex));
    }

    LOG_INFO(Client, "RemotePlayerRenderer: Pre-loaded {} trainer sprite textures.", trainer_sprites_.size());
}

void RemotePlayerRenderer::Render(const RenderContext& context) {
    
    if (!context.native_renderer || !manager_ || !context.camera || !context.queue) {
        return;
    }

    auto* renderer = static_cast<SDL_Renderer*>(context.native_renderer);

    // Ensure assets are loaded before entering the draw loop
    LoadSpritesIfNecessary(renderer);

    const auto players = manager_->GetAllPlayers();
    if (players.empty()) {
        return;
    }

    // Standardize viewport scaling math using core resolution primitives
    DynamicViewport viewport;
    viewport.Resize(Resolution{context.viewport_width, context.viewport_height}, kGbaNativeResolution);
    const ViewportRect& vrect = viewport.Rect();
    const double scale = viewport.Scale();

    const float cam_world_px = context.camera->GetX() * static_cast<float>(kTilePixelSize);
    const float cam_world_py = context.camera->GetY() * static_cast<float>(kTilePixelSize);

    const float half_native_w = static_cast<float>(kGbaNativeResolution.width) * 0.5f;
    const float half_native_h = static_cast<float>(kGbaNativeResolution.height) * 0.5f;

    const int sprite_w = static_cast<int>(16.0 * scale);
    const int sprite_h = static_cast<int>(24.0 * scale);
    const int tile_scaled_h = static_cast<int>(kTilePixelSize * scale);

    for (const auto& player : players) {
        LOG_INFO(
        Client,
        "Remote id={} world=({}, {}) camera=({}, {})",
        player.account_id,
        player.current_x,
        player.current_y,
        context.camera->GetX(),
        context.camera->GetY());
        const float world_px = player.current_x * static_cast<float>(kTilePixelSize);
        const float world_py = player.current_y * static_cast<float>(kTilePixelSize);

        // Project world coordinates relative to camera center
        const float screen_x = (world_px - cam_world_px) + half_native_w;
        const float screen_y = (world_py - cam_world_py) + half_native_h;

        // Apply viewport scale and letterbox offsets
        const float final_x = (screen_x * static_cast<float>(scale)) + static_cast<float>(vrect.x);
        const float final_y = (screen_y * static_cast<float>(scale)) + static_cast<float>(vrect.y);

        // Align the bottom of the 16x24 sprite to the bottom of the 16x16 tile
        SDL_Rect dst_rect{
            static_cast<int>(final_x),
            static_cast<int>(final_y) + tile_scaled_h - sprite_h,
            sprite_w,
            sprite_h
        };

        DrawCommand cmd;
        cmd.sort_key.layer = RenderLayerZ::kEntities;
        cmd.sort_key.sort_y = world_py; // Sort depth by Y-position for correct overlap
        cmd.dst_rect = dst_rect;

        const size_t sprite_index = !trainer_sprites_.empty() 
            ? static_cast<size_t>(player.account_id) % trainer_sprites_.size() 
            : 0;

        SDL_Texture* texture = (!trainer_sprites_.empty() && trainer_sprites_[sprite_index])
            ? trainer_sprites_[sprite_index].get()
            : nullptr;

        if (texture) {
            cmd.texture = texture;
            cmd.is_filled_rect = false;
        } else {
            // Fallback rendering: bright red box if texture asset is missing
            cmd.texture = nullptr;
            cmd.r = 255;
            cmd.g = 0;
            cmd.b = 0;
            cmd.a = 255;
            cmd.is_filled_rect = true;
        }

        context.queue->Enqueue(cmd);
    }
}

} // namespace unboundmp::render