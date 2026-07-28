#pragma once

#include "database/database.h"
#include "network/packet.h"
#include "persistence/character_service.h"
#include "persistence/inventory_service.h"
#include "persistence/pokemon_blob_service.h"
#include "persistence/story_service.h"
#include "persistence/dirty_flag_manager.h"

namespace unboundmp::persistence {

class PlayerPersistence {
public:
    static std::optional<network::PlayerDataSyncPacket> LoadPlayer(server::DatabasePool& db, uint64_t account_id);
    
    // Saves only the components marked as dirty, within a single transaction
    static bool SavePlayer(server::DatabasePool& db, 
                           uint64_t account_id, 
                           uint32_t dirty_flags,
                           const network::PlayerDataSyncPacket& state);
};

} // namespace unboundmp::persistence
