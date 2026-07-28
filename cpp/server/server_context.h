#pragma once
#include <memory>
#include <string>

namespace unboundmp::server {
    class DatabasePool;
    class AccountManager;
    class CharacterRepository;
    namespace database {
        class InventoryRepository;
    }
}

namespace unboundmp::server {
    class SessionManager;
    class PlayerRegistry;
}

namespace unboundmp::network {
    class PacketDispatcher;
    class NetworkManager;
}

namespace unboundmp::server::world {
    class WorldServer;
}

namespace unboundmp::persistence {
    class PlayerPersistence;
    class AutosaveManager;
}

namespace unboundmp::server::gameplay {
    class InventoryManager;
    class ItemManager;
    class ChatManager;
    class FriendManager;
    class TradeManager;
    class BattleManager;
}

namespace unboundmp::server {

class ServerContext {
public:
    ServerContext();
    ~ServerContext();

    void Initialize(const std::string& db_conn_str);
    void Shutdown();

    std::shared_ptr<DatabasePool> GetDatabasePool() const { return db_pool_; }
    std::shared_ptr<AccountManager> GetAccountManager() const { return account_mgr_; }
    std::shared_ptr<CharacterRepository> GetCharacterRepository() const { return character_repo_; }
    std::shared_ptr<database::InventoryRepository> GetInventoryRepository() const { return inventory_repo_; }

    std::shared_ptr<SessionManager> GetSessionManager() const { return session_mgr_; }
    std::shared_ptr<PlayerRegistry> GetPlayerRegistry() const { return player_registry_; }
    std::shared_ptr<unboundmp::network::PacketDispatcher> GetPacketDispatcher() const { return packet_dispatcher_; }
    std::shared_ptr<unboundmp::network::NetworkManager> GetNetworkManager() const { return network_mgr_; }

    std::shared_ptr<world::WorldServer> GetWorldServer() const { return world_server_; }
    
    std::shared_ptr<unboundmp::persistence::AutosaveManager> GetAutosaveManager() const { return autosave_mgr_; }

    std::shared_ptr<gameplay::InventoryManager> GetInventoryManager() const { return inventory_mgr_; }
    std::shared_ptr<gameplay::ItemManager> GetItemManager() const { return item_mgr_; }
    std::shared_ptr<gameplay::ChatManager> GetChatManager() const { return chat_mgr_; }
    std::shared_ptr<gameplay::FriendManager> GetFriendManager() const { return friend_mgr_; }
    std::shared_ptr<gameplay::TradeManager> GetTradeManager() const { return trade_mgr_; }
    std::shared_ptr<gameplay::BattleManager> GetBattleManager() const { return battle_mgr_; }

private:
    std::shared_ptr<DatabasePool> db_pool_;
    std::shared_ptr<AccountManager> account_mgr_;
    std::shared_ptr<CharacterRepository> character_repo_;
    std::shared_ptr<database::InventoryRepository> inventory_repo_;

    std::shared_ptr<SessionManager> session_mgr_;
    std::shared_ptr<PlayerRegistry> player_registry_;
    std::shared_ptr<unboundmp::network::PacketDispatcher> packet_dispatcher_;
    std::shared_ptr<unboundmp::network::NetworkManager> network_mgr_;

    std::shared_ptr<world::WorldServer> world_server_;
    
    std::shared_ptr<unboundmp::persistence::AutosaveManager> autosave_mgr_;

    std::shared_ptr<gameplay::InventoryManager> inventory_mgr_;
    std::shared_ptr<gameplay::ItemManager> item_mgr_;
    std::shared_ptr<gameplay::ChatManager> chat_mgr_;
    std::shared_ptr<gameplay::FriendManager> friend_mgr_;
    std::shared_ptr<gameplay::TradeManager> trade_mgr_;
    std::shared_ptr<gameplay::BattleManager> battle_mgr_;
};

} // namespace unboundmp::server
