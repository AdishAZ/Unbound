#include "gameplay/world_manager.h"
#include "gameplay/remote_player_manager.h"
#include "render/world_renderer.h"
#include "render/map_renderer.h"
#include "render/remote_player_renderer.h"
#include "core/game_context.h"
#include "network/network_clock.h"

#include "gameplay/entity_manager.h"

namespace unboundmp::gameplay {

WorldManager::WorldManager() = default;
WorldManager::~WorldManager() = default;

void WorldManager::Initialize(core::GameContext* ctx) {
    remote_player_mgr_ = std::make_shared<RemotePlayerManager>();
    remote_player_mgr_->Initialize(&ctx->GetNetworkClock());
    
    world_renderer_ = std::make_shared<render::WorldRenderer>();
    map_renderer_ = std::make_shared<render::MapRenderer>();
    auto remote_renderer = std::make_shared<render::RemotePlayerRenderer>(remote_player_mgr_);
    
    world_renderer_->AddLayer(map_renderer_);
    world_renderer_->AddLayer(remote_renderer);
}

void WorldManager::Shutdown() {
    world_renderer_.reset();
    map_renderer_.reset();
    remote_player_mgr_.reset();
}

void WorldManager::Update(float dt) {
    if (remote_player_mgr_) {
        remote_player_mgr_->Update(dt);
    }
}

} // namespace unboundmp::gameplay
