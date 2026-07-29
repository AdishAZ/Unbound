#pragma once

#include <memory>
#include <vector>
#include <SDL2/SDL.h>

#include "render/render_layer.h"
#include "render/sdl_texture_deleter.h"

namespace unboundmp::gameplay {
class RemotePlayerManager;
}

namespace unboundmp::render {

// RAII deleter for SDL_Texture handles


// Renders multiplayer remote player avatars (trainer sprites) into the game world.
// Integrates with RemotePlayerManager and positions sprites accurately via the camera and viewport.
class RemotePlayerRenderer : public RenderLayer {
public:
    explicit RemotePlayerRenderer(std::shared_ptr<gameplay::RemotePlayerManager> manager);
    ~RemotePlayerRenderer() override;

    // Prevent copying to ensure exclusive ownership of GPU texture handles
    RemotePlayerRenderer(const RemotePlayerRenderer&) = delete;
    RemotePlayerRenderer& operator=(const RemotePlayerRenderer&) = delete;

    // Allow default move semantics for safe container manipulation
    RemotePlayerRenderer(RemotePlayerRenderer&&) noexcept = default;
    RemotePlayerRenderer& operator=(RemotePlayerRenderer&&) noexcept = default;

    void Initialize() override;
    void Shutdown() override;

    void Render(const RenderContext& context) override;

private:
    // Helper to perform deferred, cached asset loading on the first valid frame context
    void LoadSpritesIfNecessary(SDL_Renderer* renderer);

    std::shared_ptr<gameplay::RemotePlayerManager> manager_;
    std::vector<std::unique_ptr<SDL_Texture, SDLTextureDeleter>> trainer_sprites_;
};

} // namespace unboundmp::render