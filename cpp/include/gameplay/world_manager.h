#pragma once
#include <memory>

namespace unboundmp::render {
    class WorldRenderer;
    class MapRenderer;
    class EntityRenderer;
}

namespace unboundmp::core {
    class GameContext;
}

namespace unboundmp::gameplay {
    class RemotePlayerManager;
    class EntityManager;

class WorldManager {
public:
    WorldManager();
    ~WorldManager();

    void Initialize(core::GameContext* ctx);
    void Shutdown();
    void Update(float dt);

    std::shared_ptr<render::WorldRenderer> GetWorldRenderer() const { return world_renderer_; }
    std::shared_ptr<render::MapRenderer> GetMapRenderer() const { return map_renderer_; }
    std::shared_ptr<RemotePlayerManager> GetRemotePlayerManager() const { return remote_player_mgr_; }

private:
    core::GameContext* game_context_ = nullptr;
    std::shared_ptr<RemotePlayerManager> remote_player_mgr_;
    std::shared_ptr<render::WorldRenderer> world_renderer_;
    std::shared_ptr<render::MapRenderer> map_renderer_;
};

} // namespace unboundmp::gameplay
