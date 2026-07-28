#pragma once
#include <memory>

// Forward declarations to avoid massive includes
namespace unboundmp::network {
    class MultiplayerClient;
    class ClientPacketDispatcher;
    class ClientSessionManager;
    class NetworkClock;
}

namespace unboundmp::core {
    class EventSystem;
}

namespace unboundmp::persistence {
    class ClientSaveManager;
}

namespace unboundmp::gameplay {
    class WorldManager;
    class InventoryManager;
    class ChatManager;
    class FriendManager;
    class TradeManager;
    class BattleManager;
}

namespace unboundmp::game_state {
    class GameState;
}

namespace unboundmp::gameplay {
    class SyncScheduler;
}

namespace unboundmp::parser {
    class ParserRegistry;
}

namespace unboundmp::ui {
    class UIEngine;
    class ScreenManager;
    class AssetManager;
}

namespace unboundmp::render {
    class RenderManager;
    class TileProvider;
}

// Emulation forward declaration
namespace unboundmp::emulator {
    class GameBootstrap;
}

namespace unboundmp::memory {
    class AddressTable;
}

namespace unboundmp::core {

// Centralized context passed to subsystems instead of relying on singletons or excessive arguments.
class GameContext {
public:
    GameContext();
    ~GameContext();

    void Initialize(const memory::AddressTable& addresses);
    void Update(float dt);
    void Shutdown();

    // Engine Subsystems
    network::MultiplayerClient* GetNetworkClient() const { return network_client_; }
    void SetNetworkClient(network::MultiplayerClient* client);

    network::ClientPacketDispatcher& GetPacketDispatcher() const;
    core::EventSystem& GetEventDispatcher() const;
    ui::UIEngine* GetUIEngine() const { return ui_engine_; }
    void SetUIEngine(ui::UIEngine* engine) { ui_engine_ = engine; }

    ui::ScreenManager& GetScreenManager() const;
    ui::AssetManager& GetAssetManager() const;
    render::RenderManager* GetRenderManager() const { return render_manager_; }
    void SetRenderManager(render::RenderManager* mgr) { render_manager_ = mgr; }
    
    void SetTileProvider(std::shared_ptr<render::TileProvider> provider);

    // Game Subsystems
    network::ClientSessionManager& GetSessionManager() const;
    network::NetworkClock& GetNetworkClock() const;
    persistence::ClientSaveManager& GetSaveManager() const;
    emulator::GameBootstrap& GetEmulatorBootstrap() const;

    gameplay::WorldManager* GetWorldManager() const { return world_manager_.get(); }
    game_state::GameState* GetGameState() const { return game_state_.get(); }
    
    // Future Subsystems
    gameplay::InventoryManager* GetInventoryManager() const { return inventory_manager_.get(); }
    gameplay::ChatManager* GetChatManager() const { return chat_manager_.get(); }
    gameplay::FriendManager* GetFriendManager() const { return friend_manager_.get(); }
    gameplay::TradeManager* GetTradeManager() const { return trade_manager_.get(); }
    gameplay::BattleManager* GetBattleManager() const { return battle_manager_.get(); }

    parser::ParserRegistry* GetParserRegistry() const { return parser_registry_.get(); }

private:
    network::MultiplayerClient* network_client_ = nullptr;
    ui::UIEngine* ui_engine_ = nullptr;
    render::RenderManager* render_manager_ = nullptr;
    
    std::unique_ptr<gameplay::WorldManager> world_manager_;
    std::unique_ptr<gameplay::InventoryManager> inventory_manager_;
    std::unique_ptr<gameplay::ChatManager> chat_manager_;
    std::unique_ptr<gameplay::FriendManager> friend_manager_;
    std::unique_ptr<gameplay::TradeManager> trade_manager_;
    std::unique_ptr<gameplay::BattleManager> battle_manager_;
    std::unique_ptr<gameplay::SyncScheduler> sync_scheduler_;

    std::unique_ptr<game_state::GameState> game_state_;
    std::unique_ptr<parser::ParserRegistry> parser_registry_;

    std::shared_ptr<render::TileProvider> tile_provider_;
    uint32_t current_map_id_ = 0xFFFFFFFF;
};

} // namespace unboundmp::core
