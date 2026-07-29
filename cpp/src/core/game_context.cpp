#include "core/game_context.h"
#include "network/multiplayer_client.h"
#include "network/client_packet_dispatcher.h"
#include "core/event_system.h"
#include "ui/ui_engine.h"
#include "render/render_manager.h"
#include "render/tile_provider.h"
#include "network/client_session_manager.h"
#include "network/network_clock.h"
#include "persistence/client_save_manager.h"
#include "emulator/game_bootstrap.h"
#include "gameplay/world_manager.h"
#include "gameplay/inventory_manager.h"
#include "gameplay/chat_manager.h"
#include "gameplay/friend_manager.h"
#include "gameplay/trade_manager.h"
#include "gameplay/battle_manager.h"
#include "gameplay/sync_scheduler.h"
#include "network/multiplayer_client.h"
#include "network/client_session_manager.h"
#include "game_state/game_state.h"
#include "parser/parser_registry.h"
#include "parser/coordinate_normalizer.h"
#include "memory/address_table.h"

namespace unboundmp::core {

GameContext::GameContext() = default;

GameContext::~GameContext() {
    Shutdown();
}

void GameContext::Initialize(const memory::AddressTable& addresses) {
    world_manager_ = std::make_unique<gameplay::WorldManager>();
    world_manager_->Initialize(this);
    
    inventory_manager_ = std::make_unique<gameplay::InventoryManager>();
    inventory_manager_->Initialize(this, addresses);
    
    chat_manager_ = std::make_unique<gameplay::ChatManager>();
    friend_manager_ = std::make_unique<gameplay::FriendManager>();
    trade_manager_ = std::make_unique<gameplay::TradeManager>();
    battle_manager_ = std::make_unique<gameplay::BattleManager>();

    auto& memory_api = emulator::GameBootstrap::GetInstance().GetEmulator();
    parser_registry_ = std::make_unique<parser::ParserRegistry>(memory::MemoryApi(memory_api), addresses);
    game_state_ = std::make_unique<game_state::GameState>();
}

void GameContext::Update(float dt) {
    if (auto& bootstrap = emulator::GameBootstrap::GetInstance(); bootstrap.IsBooted() && bootstrap.GetEmulator().IsRunning()) {
        if (game_state_ && parser_registry_) {
            // we should pass a frame count. For now just passing 0 or incrementing
            static int64_t frame_count = 0;
            game_state_->Update(*parser_registry_, frame_count++);
            
            auto dirty_flags = game_state_->ConsumeDirtyFlags();
            
            if ((dirty_flags & game_state::kMap) || game_state_->GetLocalPlayer().map.CombinedId() != current_map_id_) {
                auto loc = game_state_->GetLocalPlayer().map;
                if (tile_provider_) {
                    printf("[GameContext] Updating Map to %d.%d\n", loc.bank, loc.number);
                    tile_provider_->UpdateMap(static_cast<uint8_t>(loc.bank), static_cast<uint8_t>(loc.number));
                } else {
                    printf("[GameContext] Error: tile_provider_ is null!\n");
                }
                current_map_id_ = loc.CombinedId();
            }

            if (dirty_flags & game_state::kInventory) {
                if (inventory_manager_ && inventory_manager_->IsSynced() && network_client_) {
                    network::InventoryUpdatePacket inv_update;
                    
                    // A simple implementation: send the entire inventory as "changed"
                    // (Or compute a delta, but for now we just send what we have)
                    const auto& inv = game_state_->GetInventory();
                    for (const auto& item : inv.items) {
                        inv_update.changed_items.push_back(item);
                    }
                    
                    network::Packet p;
                    p.type = network::PacketType::kInventoryUpdate;
                    p.payload = inv_update.Serialize();
                    if (auto session = network::ClientSessionManager::GetInstance().GetSession()) {
                        p.session_token = session->session_token;
                        network_client_->SendPacket(p);
                    }
                }
            }
            
            if (render_manager_) {
                float px = static_cast<float>(game_state_->GetLocalPlayer().position.x);
                float py = static_cast<float>(game_state_->GetLocalPlayer().position.y);
                
                render_manager_->GetCameraController().SetTarget(px, py);
            }

            if (sync_scheduler_) {
                sync_scheduler_->Tick();
            }
        }
        if (world_manager_) {
            world_manager_->Update(dt);
        }
    }
}

void GameContext::SetTileProvider(std::shared_ptr<render::TileProvider> provider) {
    tile_provider_ = provider;
    if (render_manager_) {
        render_manager_->GetCameraController().SetTileProvider(provider.get());
    }
}

void GameContext::SetNetworkClient(network::MultiplayerClient* client) {
    network_client_ = client;
    if (network_client_ && game_state_) {
        sync_scheduler_ = std::make_unique<gameplay::SyncScheduler>(
            game_state_.get(),
            [this](const network::Packet& p) {
                if (network_client_) {
                    // Create a copy of packet to append session token
                    network::Packet to_send = p;
                    if (auto session = network::ClientSessionManager::GetInstance().GetSession()) {
                        to_send.session_token = session->session_token;
                    }
                    network_client_->SendPacket(to_send);
                }
            }
        );
    }
}

void GameContext::Shutdown() {
    if (world_manager_) {
        world_manager_->Shutdown();
        world_manager_.reset();
    }
    inventory_manager_.reset();
    chat_manager_.reset();
    friend_manager_.reset();
    trade_manager_.reset();
    battle_manager_.reset();
}

network::ClientPacketDispatcher& GameContext::GetPacketDispatcher() const {
    return network::ClientPacketDispatcher::GetInstance();
}

core::EventSystem& GameContext::GetEventDispatcher() const {
    return core::EventSystem::GetInstance();
}

ui::ScreenManager& GameContext::GetScreenManager() const {
    return ui_engine_->GetScreens();
}

ui::AssetManager& GameContext::GetAssetManager() const {
    return ui_engine_->GetAssetManager();
}

network::ClientSessionManager& GameContext::GetSessionManager() const {
    return network::ClientSessionManager::GetInstance();
}

network::NetworkClock& GameContext::GetNetworkClock() const {
    static network::NetworkClock clock;
    return clock;
}

persistence::ClientSaveManager& GameContext::GetSaveManager() const {
    return persistence::ClientSaveManager::GetInstance();
}

emulator::GameBootstrap& GameContext::GetEmulatorBootstrap() const {
    return emulator::GameBootstrap::GetInstance();
}

} // namespace unboundmp::core
