#include "render/render_queue.h"

#include <algorithm>
#include <ranges>
#include <span>
#include <SDL2/SDL.h>

namespace unboundmp::render {

namespace {

// RAII helper to guarantee blend mode restoration after primitive rendering
class BlendModeGuard {
public:
    BlendModeGuard(SDL_Renderer* renderer, bool needs_blending)
        : renderer_(renderer), needs_blending_(needs_blending) {
        if (needs_blending_) {
            SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        }
    }

    ~BlendModeGuard() {
        if (needs_blending_) {
            SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
        }
    }

    BlendModeGuard(const BlendModeGuard&) = delete;
    BlendModeGuard& operator=(const BlendModeGuard&) = delete;

private:
    SDL_Renderer* renderer_;
    bool needs_blending_;
};

} // namespace

void RenderQueue::Enqueue(const DrawCommand& cmd) {
    commands_.push_back(cmd);
}

void RenderQueue::Flush(SDL_Renderer* renderer) {
    if (!renderer || commands_.empty()) {
        return;
    }

    last_command_count_ = commands_.size();
    last_batch_count_ = 0;

    // Stage 1: Sort commands deterministically by layer, Y-order, and sub-priority[cite: 14, 38]
    const Uint64 start_time = SDL_GetPerformanceCounter();

    std::sort(commands_.begin(), commands_.end(),
    [](const DrawCommand& a, const DrawCommand& b)
    {
        return a.sort_key < b.sort_key;
    });

    const Uint64 end_time = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    last_sort_time_ms_ = (frequency > 0.0)
        ? static_cast<float>((end_time - start_time) * 1000.0 / frequency)
        : 0.0f;

    // Stage 2: Execute centralized rendering pipeline[cite: 14]
    SDL_Texture* last_texture = nullptr;
    uint8_t last_r = 255, last_g = 255, last_b = 255, last_a = 255;
    bool is_first_command = true;

    for (const DrawCommand& cmd : commands_) {
        // Track state transitions to calculate hardware batch breaks[cite: 14]
        const bool state_changed = is_first_command ||
                                   cmd.texture != last_texture ||
                                   cmd.r != last_r ||
                                   cmd.g != last_g ||
                                   cmd.b != last_b ||
                                   cmd.a != last_a;

        if (state_changed) {
            last_batch_count_++;
            last_texture = cmd.texture;
            last_r = cmd.r;
            last_g = cmd.g;
            last_b = cmd.b;
            last_a = cmd.a;
            is_first_command = false;
        }

        // Path A: Arbitrary Geometry & Mesh Rendering[cite: 14, 38]
        if (cmd.vertices && cmd.num_vertices > 0) {
            const bool requires_transform = (cmd.dx != 0.0f || cmd.dy != 0.0f || cmd.scale != 1.0f);

            if (requires_transform) {
                // Ensure staging buffer capacity without shrinking or zero-filling[cite: 14]
                if (temp_vertices_.size() < static_cast<size_t>(cmd.num_vertices)) {
                    temp_vertices_.resize(cmd.num_vertices);
                }

                const std::span<const SDL_Vertex> src_verts(cmd.vertices, cmd.num_vertices);
                for (size_t i = 0; i < src_verts.size(); ++i) {
                    temp_vertices_[i] = src_verts[i];
                    temp_vertices_[i].position.x = src_verts[i].position.x * cmd.scale + cmd.dx;
                    temp_vertices_[i].position.y = src_verts[i].position.y * cmd.scale + cmd.dy;
                }

                SDL_RenderGeometry(
                    renderer,
                    cmd.texture,
                    temp_vertices_.data(),
                    cmd.num_vertices,
                    cmd.indices,
                    cmd.num_indices
                );
            } else {
                SDL_RenderGeometry(
                    renderer,
                    cmd.texture,
                    cmd.vertices,
                    cmd.num_vertices,
                    cmd.indices,
                    cmd.num_indices
                );
            }
        }
        // Path B: Textured Quad Copy[cite: 14, 38]
        else if (cmd.texture) {
            SDL_SetTextureColorMod(cmd.texture, cmd.r, cmd.g, cmd.b);
            SDL_SetTextureAlphaMod(cmd.texture, cmd.a);

            const SDL_Rect* src_ptr = (cmd.src_rect.w > 0 && cmd.src_rect.h > 0)
                ? &cmd.src_rect
                : nullptr;

            SDL_RenderCopy(renderer, cmd.texture, src_ptr, &cmd.dst_rect);
        }
        // Path C: Primitive Shape Drawing (Filled/Outlined Rectangles)[cite: 14, 38]
        else {
            SDL_SetRenderDrawColor(renderer, cmd.r, cmd.g, cmd.b, cmd.a);

            const BlendModeGuard blend_guard(renderer, cmd.a < 255);

            if (cmd.is_filled_rect) {
                SDL_RenderFillRect(renderer, &cmd.dst_rect);
            } else {
                SDL_RenderDrawRect(renderer, &cmd.dst_rect);
            }
        }
    }

    Clear();
}

void RenderQueue::Clear() {
    commands_.clear();
}

} // namespace unboundmp::render