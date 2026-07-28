#include "persistence/player_persistence.h"
#include <iostream>

namespace unboundmp::persistence {

std::optional<network::PlayerDataSyncPacket> PlayerPersistence::LoadPlayer(server::DatabasePool& db, uint64_t account_id) {
    network::PlayerDataSyncPacket sync;
    sync.account_id = account_id;
    
    bool found = false;
    
    db.ExecuteTransaction([&](pqxx::work& txn) {
        auto character = CharacterService::GetByAccountId(txn, account_id);
        if (!character) return false;
        
        found = true;
        
        sync.map_id = character->map_id;
        sync.x = character->x;
        sync.y = character->y;
        sync.direction = character->direction;
        sync.money = character->money;
        
        auto inventory = InventoryService::Get(txn, character->id);
        if (inventory) sync.inventory_blob = *inventory;
        
        auto party = PokemonBlobService::GetParty(txn, character->id);
        if (party) sync.party_slots = *party;
        
        auto pc = PokemonBlobService::GetPc(txn, character->id);
        if (pc) sync.pc_blob = *pc;
        
        auto story = StoryService::Get(txn, character->id);
        if (story) {
            sync.story_flags = story->flags;
            sync.story_badges = story->badges;
            sync.story_quests = story->quests;
        }
        
        return true;
    });
    
    if (!found) return std::nullopt;
    
    return sync;
}

bool PlayerPersistence::SavePlayer(server::DatabasePool& db, 
                                   uint64_t account_id, 
                                   uint32_t dirty_flags,
                                   const network::PlayerDataSyncPacket& state) {
    if (dirty_flags == 0) return true; // Nothing to save
    
    return db.ExecuteTransaction([&](pqxx::work& txn) {
        // First get the character ID
        auto character = CharacterService::GetByAccountId(txn, account_id);
        if (!character) return false; // Character must exist
        
        if (dirty_flags & static_cast<uint32_t>(DirtyComponent::kWorldState) ||
            dirty_flags & static_cast<uint32_t>(DirtyComponent::kMoney)) {
            character->map_id = state.map_id;
            character->x = state.x;
            character->y = state.y;
            character->direction = state.direction;
            character->money = state.money;
            // Update last_login or playtime if needed?
            if (!CharacterService::Update(txn, *character)) return false;
        }
        
        if (dirty_flags & static_cast<uint32_t>(DirtyComponent::kInventory)) {
            if (!InventoryService::Update(txn, character->id, state.inventory_blob)) return false;
        }
        
        if (dirty_flags & static_cast<uint32_t>(DirtyComponent::kParty)) {
            if (!PokemonBlobService::UpdateParty(txn, character->id, state.party_slots)) return false;
        }
        
        if (dirty_flags & static_cast<uint32_t>(DirtyComponent::kPc)) {
            if (!PokemonBlobService::UpdatePc(txn, character->id, state.pc_blob)) return false;
        }
        
        if (dirty_flags & static_cast<uint32_t>(DirtyComponent::kStory)) {
            memory::StoryData sd;
            sd.flags = state.story_flags;
            sd.badges = state.story_badges;
            sd.quests = state.story_quests;
            if (!StoryService::Update(txn, character->id, sd)) return false;
        }
        
        return true; // Commit if all successful
    });
}

} // namespace unboundmp::persistence
