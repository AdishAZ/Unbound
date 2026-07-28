#include "gameplay/item_manager.h"
#include <iostream>

namespace unboundmp::server::gameplay {

ItemManager::ItemManager(std::shared_ptr<InventoryManager> inventory_manager)
    : inventory_manager_(std::move(inventory_manager)) {}

bool ItemManager::ValidateUse(uint64_t character_id, uint32_t slot_index, uint32_t target_entity) {
    unboundmp::models::Inventory inv;
    if (!inventory_manager_->GetInventory(character_id, inv)) return false;
    
    auto item_opt = inv.GetItemAt(slot_index);
    if (!item_opt) return false;
    
    // For now, any medicine or berry can be used. Key items cannot be consumed, etc.
    if (item_opt->type == unboundmp::models::ItemType::kKeyItem) return false;
    
    return true;
}

bool ItemManager::ConsumeItem(uint64_t character_id, uint32_t slot_index, uint32_t target_entity) {
    if (!ValidateUse(character_id, slot_index, target_entity)) return false;
    
    unboundmp::models::Inventory inv;
    if (!inventory_manager_->GetInventory(character_id, inv)) return false;
    auto item_opt = inv.GetItemAt(slot_index);
    
    // Apply effect (stubbed)
    std::cout << "[INFO] Character " << character_id << " used item " << item_opt->name << " on target " << target_entity << "\n";
    
    // Decrease quantity
    return inventory_manager_->RemoveItem(character_id, slot_index, 1);
}

} // namespace unboundmp::server::gameplay
