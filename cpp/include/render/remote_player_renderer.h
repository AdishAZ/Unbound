#pragma once

#include "render/render_layer.h"
#include <memory>
#include <SDL.h>
#include <vector>

namespace unboundmp::gameplay {
    class RemotePlayerManager;
}

namespace unboundmp::render {

class RemotePlayerRenderer : public RenderLayer {
public:
    explicit RemotePlayerRenderer(std::shared_ptr<gameplay::RemotePlayerManager> manager);
    ~RemotePlayerRenderer() override;

    void Initialize() override;
    void Shutdown() override;

    void Render(const RenderContext& context) override;

private:
    std::shared_ptr<gameplay::RemotePlayerManager> manager_;
    std::vector<SDL_Texture*> trainer_sprites_;
};

} // namespace unboundmp::render
