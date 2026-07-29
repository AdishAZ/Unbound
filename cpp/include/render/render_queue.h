#pragma once

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>

namespace unboundmp::render {

enum class RenderLayerZ : int {
    kBackground = 0,
    kGround,
    kGroundDecorations,
    kLowObjects,
    kEntities,
    kHighObjects,
    kRoofs,
    kTrees,
    kWeather,
    kLighting,
    kUI,
    kDebug
};

struct RenderSortKey {
    RenderLayerZ layer = RenderLayerZ::kBackground;
    float sort_y = 0.0f;
    int32_t sub_priority = 0;

    // Optimized sorting predicate: constexpr and noexcept to allow strict compiler inlining
    // during the critical std::ranges::sort pass.
    [[nodiscard]] constexpr bool operator<(const RenderSortKey& other) const noexcept {
        if (layer != other.layer) {
            return layer < other.layer;
        }
        if (sort_y != other.sort_y) {
            return sort_y < other.sort_y;
        }
        return sub_priority < other.sub_priority;
    }
};

struct DrawCommand {
    RenderSortKey sort_key{};
    
    SDL_Texture* texture = nullptr;
    SDL_Rect src_rect{0, 0, 0, 0};
    SDL_Rect dst_rect{0, 0, 0, 0};
    
    // Geometry rendering
    const SDL_Vertex* vertices = nullptr;
    int num_vertices = 0;
    const int* indices = nullptr;
    int num_indices = 0;
    
    // Transform for geometry
    float dx = 0.0f;
    float dy = 0.0f;
    float scale = 1.0f;
    
    // For colored rects if texture is null and vertices is null
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    bool is_filled_rect = true;
};

class RenderQueue {
public:
    RenderQueue() = default;
    ~RenderQueue() = default;

    // Prevent expensive accidental deep copies of the internal command/vertex buffers.
    RenderQueue(const RenderQueue&) = delete;
    RenderQueue& operator=(const RenderQueue&) = delete;

    // Allow default move semantics
    RenderQueue(RenderQueue&&) noexcept = default;
    RenderQueue& operator=(RenderQueue&&) noexcept = default;

    void Enqueue(const DrawCommand& cmd);
    
    // Sorts commands by RenderSortKey and flushes them to the hardware backend
    void Flush(SDL_Renderer* renderer);
    
    void Clear();

    [[nodiscard]] size_t GetCommandCount() const noexcept { return last_command_count_; }
    [[nodiscard]] size_t GetBatchCount() const noexcept { return last_batch_count_; }
    [[nodiscard]] float GetSortTimeMs() const noexcept { return last_sort_time_ms_; }

private:
    std::vector<DrawCommand> commands_;
    std::vector<SDL_Vertex> temp_vertices_; // Preserved across frames to prevent CPU re-allocations
    
    size_t last_command_count_ = 0;
    size_t last_batch_count_ = 0;
    float last_sort_time_ms_ = 0.0f;
};

} // namespace unboundmp::render