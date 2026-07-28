#pragma once

namespace unboundmp::render {

class Camera; // Forward declare
class RenderQueue;
struct RenderContext {
    RenderQueue* queue = nullptr;
    const Camera* camera = nullptr;
    int viewport_width = 0;
    int viewport_height = 0;
    float delta_time = 0.0f;
    void* native_renderer = nullptr; // e.g. SDL_Renderer*
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    
    // Draw using the provided render context
    virtual void Render(const RenderContext& context) = 0;
};

} // namespace unboundmp::render
