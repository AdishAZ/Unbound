#include "server_context.h"
#include "database/database.h"
#include "accounts/account_manager.h"
#include "database/character_repository.h"
#include "database/inventory_repository.h"

#include "sessions/session_manager.h"
#include "network/player_registry.h"
#include "network/packet_dispatcher.h"
#include "network/network_manager.h"

#include "world/world_server.h"

#include "persistence/player_persistence.h"
#include "persistence/autosave_manager.h"

#include "gameplay/inventory_manager.h"
#include "gameplay/item_manager.h"
#include "gameplay/chat_manager.h"
#include "gameplay/friend_manager.h"
#include "gameplay/trade_manager.h"
#include "gameplay/battle_manager.h"

namespace unboundmp::server {

ServerContext::ServerContext() = default;
ServerContext::~ServerContext() { Shutdown(); }

void ServerContext::Initialize(const std::string& db_conn_str) {
    db_pool_ = std::make_shared<DatabasePool>(db_conn_str, 4);
    
    account_mgr_ = std::make_shared<AccountManager>(db_pool_);
    character_repo_ = std::make_shared<CharacterRepository>(db_pool_);
    inventory_repo_ = std::make_shared<database::InventoryRepository>(db_pool_);
    
    session_mgr_ = std::make_shared<SessionManager>();
    player_registry_ = std::make_shared<PlayerRegistry>();
    packet_dispatcher_ = std::make_shared<unboundmp::network::PacketDispatcher>();
    network_mgr_ = std::make_shared<unboundmp::network::NetworkManager>();
    
    world_server_ = std::make_shared<world::WorldServer>();
    world_server_->Initialize();
    
    // Pass packet_dispatcher_ to NetworkManager (it handles its own registration but maybe takes it in constructor?)
    // Actually NetworkManager doesn't take packet_dispatcher in its constructor. 
    
    autosave_mgr_ = std::make_shared<unboundmp::persistence::AutosaveManager>(
        std::chrono::seconds(10),
        [this]() {
            for (const auto& player : player_registry_->GetAllPlayers()) {
                if (inventory_mgr_ && inventory_mgr_->IsDirty(player.character_id)) {
                    inventory_mgr_->SaveInventory(player.character_id);
                }
                if (player.dirty_manager && player.dirty_manager->AnyDirty()) {
                    uint32_t flags = player.dirty_manager->GetFlagsAndClear();
                    if (unboundmp::persistence::PlayerPersistence::SavePlayer(*db_pool_, player.account_id, flags, *player.persistence_state)) {
                        auto session = session_mgr_->GetSessionByToken(player.session_token);
                        if (session && session->connection) {
                            unboundmp::network::AutosaveAckPacket ack;
                            ack.account_id = player.account_id;
                            unboundmp::network::Packet p;
                            p.type = unboundmp::network::PacketType::kAutosaveAck;
                            p.payload = ack.Serialize();
                            session->connection->SendPacket(p);
                        }
                    }
                }
            }
        }
    );
    autosave_mgr_->Start();
    inventory_mgr_ = std::make_shared<gameplay::InventoryManager>(inventory_repo_);
    item_mgr_ = std::make_shared<gameplay::ItemManager>(inventory_mgr_);
    chat_mgr_ = std::make_shared<gameplay::ChatManager>();
    friend_mgr_ = std::make_shared<gameplay::FriendManager>();
    trade_mgr_ = std::make_shared<gameplay::TradeManager>();
    battle_mgr_ = std::make_shared<gameplay::BattleManager>();
}

void ServerContext::Shutdown() {
    if (autosave_mgr_) autosave_mgr_->Stop();
    if (network_mgr_) network_mgr_->StopServer();
    if (world_server_) world_server_->Shutdown();
    
    autosave_mgr_.reset();
    world_server_.reset();
    network_mgr_.reset();
    packet_dispatcher_.reset();
    player_registry_.reset();
    session_mgr_.reset();
    item_mgr_.reset();
    inventory_repo_.reset();
    character_repo_.reset();
    account_mgr_.reset();
    db_pool_.reset();
    
    inventory_mgr_.reset();
    chat_mgr_.reset();
    friend_mgr_.reset();
    trade_mgr_.reset();
    battle_mgr_.reset();
}

} // namespace unboundmp::server
