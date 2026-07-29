#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include <algorithm>

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

    bool operator<(const RenderSortKey& other) const {
        if (layer != other.layer) return static_cast<int>(layer) < static_cast<int>(other.layer);
        if (sort_y != other.sort_y) return sort_y < other.sort_y;
        return sub_priority < other.sub_priority;
    }
};

struct DrawCommand {
    RenderSortKey sort_key;
    
    SDL_Texture* texture = nullptr;
    SDL_Rect src_rect = {0, 0, 0, 0};
    SDL_Rect dst_rect = {0, 0, 0, 0};
    
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

    void Enqueue(const DrawCommand& cmd);
    
    // Sorts commands and issues them to the renderer
    void Flush(SDL_Renderer* renderer);
    
    void Clear();

    size_t GetCommandCount() const { return last_command_count_; }
    size_t GetBatchCount() const { return last_batch_count_; }
    float GetSortTimeMs() const { return last_sort_time_ms_; }

private:
    std::vector<DrawCommand> commands_;
    std::vector<SDL_Vertex> temp_vertices_; // For transforming geometry
    
    size_t last_command_count_ = 0;
    size_t last_batch_count_ = 0;
    float last_sort_time_ms_ = 0.0f;
};

} // namespace unboundmp::render
