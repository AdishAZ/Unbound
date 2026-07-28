#include "render/remote_player_renderer.h"
#include "gameplay/remote_player_manager.h"
#include "render/camera.h"
#include "core/log_manager.h"
#include "render/render_queue.h"

#include <string>
#include <iostream>

namespace unboundmp::render {

RemotePlayerRenderer::RemotePlayerRenderer(std::shared_ptr<gameplay::RemotePlayerManager> manager)
    : RenderLayer("RemotePlayerRenderer"), manager_(std::move(manager)) {}

RemotePlayerRenderer::~RemotePlayerRenderer() {
    Shutdown();
}

void RemotePlayerRenderer::Initialize() {
}

void RemotePlayerRenderer::Shutdown() {
    for (auto tex : trainer_sprites_) {
        if (tex) {
            SDL_DestroyTexture(tex);
        }
    }
    trainer_sprites_.clear();
}

void RemotePlayerRenderer::Render(const RenderContext& context) {
    if (!context.native_renderer || !manager_ || !context.camera || !context.queue) return;
    
    SDL_Renderer* renderer = static_cast<SDL_Renderer*>(context.native_renderer);
    
    // Load sprites if not loaded
    if (trainer_sprites_.empty()) {
        for (int i = 0; i < 10; ++i) {
            std::string path = "assets/characters/trainer_" + std::to_string(i) + ".bmp";
            SDL_Surface* surface = SDL_LoadBMP(path.c_str());
            if (surface) {
                SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                trainer_sprites_.push_back(tex);
                if (!tex) {
                    std::cerr << "[RemotePlayerRenderer] Failed to create texture for " << path << ": " << SDL_GetError() << std::endl;
                }
            } else {
                std::cerr << "[RemotePlayerRenderer] Failed to load BMP: " << path << ": " << SDL_GetError() << std::endl;
                trainer_sprites_.push_back(nullptr);
            }
        }
    }
    
    auto players = manager_->GetAllPlayers();
    
    float scale_x = static_cast<float>(context.viewport_width) / 240.0f;
    float scale_y = static_cast<float>(context.viewport_height) / 160.0f;
    float scale = std::min(scale_x, scale_y);
    
    for (const auto& p : players) {
        float world_px = p.current_x * 16.0f;
        float world_py = p.current_y * 16.0f;
        
        float cam_x = context.camera->GetX() * 16.0f;
        float cam_y = context.camera->GetY() * 16.0f;
        
        float screen_x = (world_px - cam_x) + (240.0f / 2.0f);
        float screen_y = (world_py - cam_y) + (160.0f / 2.0f);
        
        float final_x = (screen_x * scale) + (context.viewport_width - (240.0f * scale)) / 2.0f;
        float final_y = (screen_y * scale) + (context.viewport_height - (160.0f * scale)) / 2.0f;
        
        // Sprite is 16x24, bottom aligned to the 16x16 tile
        int sprite_w = static_cast<int>(16 * scale);
        int sprite_h = static_cast<int>(24 * scale);
        
        SDL_Rect dst_rect;
        dst_rect.x = static_cast<int>(final_x); // sprite is 16w, tile is 16w, so x aligns perfectly
        dst_rect.y = static_cast<int>(final_y) + static_cast<int>(16 * scale) - sprite_h; // align bottoms
        dst_rect.w = sprite_w;
        dst_rect.h = sprite_h;
        
        uint32_t sprite_index = p.account_id % trainer_sprites_.size();
        SDL_Texture* tex = trainer_sprites_[sprite_index];
        
        if (tex) {
            DrawCommand cmd;
            cmd.sort_key.layer = RenderLayerZ::kEntities; // Draw over the map
            cmd.texture = tex;
            cmd.dst_rect = dst_rect;
            context.queue->Enqueue(cmd);
        } else {
            DrawCommand cmd;
            cmd.sort_key.layer = RenderLayerZ::kEntities;
            cmd.dst_rect = dst_rect;
            cmd.r = 255; cmd.g = 0; cmd.b = 0; cmd.a = 255;
            cmd.is_filled_rect = true;
            context.queue->Enqueue(cmd);
        }
    }
}

} // namespace unboundmp::render
