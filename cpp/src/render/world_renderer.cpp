#include "render/world_renderer.h"

#include <algorithm>
#include <ranges>

namespace unboundmp::render {

WorldRenderer::WorldRenderer() : RenderLayer("WorldRenderer") {}

void WorldRenderer::Initialize() {
    for (const auto& layer : layers_) {
        if (layer) {
            layer->Initialize();
        }
    }
}

void WorldRenderer::Shutdown() {
    for (const auto& layer : layers_) {
        if (layer) {
            layer->Shutdown();
        }
    }
    
    // Explicitly release resources to satisfy RAII teardown[cite: 19, 39]
    layers_.clear();
}

void WorldRenderer::AddLayer(std::shared_ptr<IRenderer> layer) {
    if (!layer) {
        return;
    }

    // Ensure idempotency: prevent duplicate layers from inflating the render pipeline[cite: 19]
    if (std::ranges::find(layers_, layer) == layers_.end()) {
        layers_.push_back(std::move(layer));
    }
}

void WorldRenderer::Render(const RenderContext& context) {
    // Prune any expired or null layers before iterating to guarantee pipeline safety[cite: 19, 39]
    std::erase_if(layers_, [](const std::shared_ptr<IRenderer>& l) {
        return !l;
    });

    for (const auto& layer : layers_) {
        layer->Render(context);
    }
}

} // namespace unboundmp::render