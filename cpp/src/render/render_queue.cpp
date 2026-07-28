#include "render/render_queue.h"

namespace unboundmp::render {

void RenderQueue::Enqueue(const DrawCommand& cmd) {
    commands_.push_back(cmd);
}

void RenderQueue::Flush(SDL_Renderer* renderer) {
    if (!renderer) return;

    if (commands_.empty()) return;
    
    last_command_count_ = commands_.size();
    last_batch_count_ = 0;
    
    Uint64 start_time = SDL_GetPerformanceCounter();

    // Sort commands using the RenderSortKey operator<
    std::sort(commands_.begin(), commands_.end(), [](const DrawCommand& a, const DrawCommand& b) {
        return a.sort_key < b.sort_key;
    });
    
    Uint64 end_time = SDL_GetPerformanceCounter();
    last_sort_time_ms_ = static_cast<float>(end_time - start_time) * 1000.0f / static_cast<float>(SDL_GetPerformanceFrequency());

    SDL_Texture* last_texture = nullptr;
    uint8_t last_r = 255, last_g = 255, last_b = 255, last_a = 255;

    for (const auto& cmd : commands_) {
        // Track batches conceptually (every state change is a new batch)
        bool state_changed = false;
        if (cmd.texture != last_texture || cmd.r != last_r || cmd.g != last_g || cmd.b != last_b || cmd.a != last_a) {
            state_changed = true;
            last_texture = cmd.texture;
            last_r = cmd.r; last_g = cmd.g; last_b = cmd.b; last_a = cmd.a;
        }
        if (state_changed) {
            last_batch_count_++;
        }
        
        if (cmd.vertices && cmd.num_vertices > 0) {
            if (cmd.dx != 0.0f || cmd.dy != 0.0f || cmd.scale != 1.0f) {
                temp_vertices_.resize(cmd.num_vertices);
                for (int i = 0; i < cmd.num_vertices; ++i) {
                    temp_vertices_[i] = cmd.vertices[i];
                    temp_vertices_[i].position.x = cmd.vertices[i].position.x * cmd.scale + cmd.dx;
                    temp_vertices_[i].position.y = cmd.vertices[i].position.y * cmd.scale + cmd.dy;
                }
                SDL_RenderGeometry(renderer, cmd.texture, temp_vertices_.data(), cmd.num_vertices, cmd.indices, cmd.num_indices);
            } else {
                SDL_RenderGeometry(renderer, cmd.texture, cmd.vertices, cmd.num_vertices, cmd.indices, cmd.num_indices);
            }
        }
        else if (cmd.texture) {
            SDL_SetTextureColorMod(cmd.texture, cmd.r, cmd.g, cmd.b);
            SDL_SetTextureAlphaMod(cmd.texture, cmd.a);
            
            const SDL_Rect* src = (cmd.src_rect.w > 0 && cmd.src_rect.h > 0) ? &cmd.src_rect : nullptr;
            SDL_RenderCopy(renderer, cmd.texture, src, &cmd.dst_rect);
        } else {
            SDL_SetRenderDrawColor(renderer, cmd.r, cmd.g, cmd.b, cmd.a);
            if (cmd.a < 255) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            }
            
            if (cmd.is_filled_rect) {
                SDL_RenderFillRect(renderer, &cmd.dst_rect);
            } else {
                SDL_RenderDrawRect(renderer, &cmd.dst_rect);
            }
            
            if (cmd.a < 255) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
        }
    }
    
    Clear();
}

void RenderQueue::Clear() {
    commands_.clear();
}

} // namespace unboundmp::render
