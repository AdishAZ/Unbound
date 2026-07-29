#pragma once

#include <memory>
#include <vector>

#include "render/render_layer.h"

namespace unboundmp::render {

// Orchestrates the composite rendering of the game world (e.g., Map + Entities).
// Acts as a pipeline node that delegates to ordered sub-layers.
class WorldRenderer : public RenderLayer {
public:
    WorldRenderer();
    ~WorldRenderer() override = default;

    // Prevent accidental shallow copies of the composite layer stack
    WorldRenderer(const WorldRenderer&) = delete;
    WorldRenderer& operator=(const WorldRenderer&) = delete;

    // Allow default move semantics for safe transfer
    WorldRenderer(WorldRenderer&&) noexcept = default;
    WorldRenderer& operator=(WorldRenderer&&) noexcept = default;

    // Lifecycle management for the world layer and all children
    void Initialize() override;
    void Shutdown() override;

    // Injects a new layer into the world rendering pipeline
    void AddLayer(std::shared_ptr<IRenderer> layer);

    // Dispatches the render context to all active, non-null child layers
    void Render(const RenderContext& context) override;

private:
    std::vector<std::shared_ptr<IRenderer>> layers_;
};

} // namespace unboundmp::render