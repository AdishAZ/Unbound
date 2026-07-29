#pragma once

#include "render/render_layer.h"
#include <memory>
#include <vector>

namespace unboundmp::render {

// Orchestrates the rendering of the game world (Map + Entities)
class WorldRenderer : public RenderLayer {
public:
    WorldRenderer();
    ~WorldRenderer() override = default;

    void Initialize() override;
    void Shutdown() override;

    void AddLayer(std::shared_ptr<IRenderer> layer);

    void Render(const RenderContext& context) override;

private:
    std::vector<std::shared_ptr<IRenderer>> layers_;
};

} // namespace unboundmp::render
