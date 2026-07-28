#include "render/world_renderer.h"

namespace unboundmp::render {

WorldRenderer::WorldRenderer() : RenderLayer("WorldRenderer") {}

void WorldRenderer::Initialize() {
    for (auto& layer : layers_) {
        layer->Initialize();
    }
}

void WorldRenderer::Shutdown() {
    for (auto& layer : layers_) {
        layer->Shutdown();
    }
    layers_.clear();
}

void WorldRenderer::AddLayer(std::shared_ptr<IRenderer> layer) {
    if (layer) {
        layers_.push_back(layer);
    }
}

void WorldRenderer::Render(const RenderContext& context) {
    for (auto& layer : layers_) {
        layer->Render(context);
    }
}

} // namespace unboundmp::render
