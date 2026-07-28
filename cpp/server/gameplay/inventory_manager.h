#pragma once

#include "models/inventory.h"
#include "database/inventory_repository.h"
#include <memory>
#include <unordered_map>
#include <mutex>

namespace unboundmp::server::gameplay {

class InventoryManager {
public:
    explicit InventoryManager(std::shared_ptr<database::InventoryRepository> repo);
    ~InventoryManager() = default;

    // Load inventory from DB into memory cache
    void LoadInventory(uint64_t character_id);
    
    // Save inventory from memory cache to DB
    void SaveInventory(uint64_t character_id);

    // Remove from memory cache (e.g., on logout)
    void UnloadInventory(uint64_t character_id);

    // Get a copy of the inventory
    bool GetInventory(uint64_t character_id, unboundmp::models::Inventory& out_inventory);

    // Moves an item from one slot to another (handles swapping and stacking)
    // Returns true if successful, false if invalid
    bool MoveItem(uint64_t character_id, uint32_t from_slot, uint32_t to_slot, uint32_t amount);

    // Adds an item to the inventory (stacking if possible, or finding a free slot)
    bool AddItem(uint64_t character_id, const unboundmp::models::Item& item);

    // Removes an item/amount from a specific slot
    bool RemoveItem(uint64_t character_id, uint32_t slot_index, uint32_t amount);

    // Modifies money
    bool AddMoney(uint64_t character_id, uint64_t amount);
    bool RemoveMoney(uint64_t character_id, uint64_t amount);
    
    // Check if dirty
    bool IsDirty(uint64_t character_id);
    void ClearDirty(uint64_t character_id);

private:
    std::shared_ptr<database::InventoryRepository> repo_;
    std::unordered_map<uint64_t, unboundmp::models::Inventory> cache_;
    std::mutex cache_mutex_;
};

} // namespace unboundmp::server::gameplay
