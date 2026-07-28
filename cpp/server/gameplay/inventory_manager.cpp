#include "gameplay/inventory_manager.h"
#include <iostream>

namespace unboundmp::server::gameplay {

InventoryManager::InventoryManager(std::shared_ptr<database::InventoryRepository> repo)
    : repo_(std::move(repo)) {}

void InventoryManager::LoadInventory(uint64_t character_id) {
    auto inv = repo_->LoadInventory(character_id);
    
    // Add default items if the inventory is empty for testing
    if (inv.items.empty()) {
        unboundmp::models::Item master_ball;
        master_ball.id = 1; // Master Ball
        master_ball.quantity = 99;
        master_ball.slot_index = 0;
        master_ball.stack_size = 99;
        master_ball.type = unboundmp::models::ItemType::kPokeball;
        master_ball.name = "Master Ball";
        
        unboundmp::models::Item potion;
        potion.id = 13; // Potion
        potion.quantity = 50;
        potion.slot_index = 1;
        potion.stack_size = 99;
        potion.type = unboundmp::models::ItemType::kMedicine;
        potion.name = "Potion";
        
        inv.items.push_back(master_ball);
        inv.items.push_back(potion);
        inv.occupied_slots = 2;
        inv.max_slots = 100; // Unbound expands pockets, give enough max slots
        inv.is_dirty = true;
    }

    std::lock_guard<std::mutex> lock(cache_mutex_);
    cache_[character_id] = inv;
}

void InventoryManager::SaveInventory(uint64_t character_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it != cache_.end()) {
        repo_->SaveInventory(character_id, it->second);
        it->second.is_dirty = false;
    }
}

void InventoryManager::UnloadInventory(uint64_t character_id) {
    SaveInventory(character_id); // Ensure we save before unloading
    
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cache_.erase(character_id);
}

bool InventoryManager::GetInventory(uint64_t character_id, unboundmp::models::Inventory& out_inventory) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it != cache_.end()) {
        out_inventory = it->second;
        return true;
    }
    return false;
}

bool InventoryManager::MoveItem(uint64_t character_id, uint32_t from_slot, uint32_t to_slot, uint32_t amount) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it == cache_.end()) return false;
    
    auto& inv = it->second;
    if (from_slot >= inv.max_slots || to_slot >= inv.max_slots) return false;
    if (from_slot == to_slot) return false;

    // Find the items
    unboundmp::models::Item* from_item = nullptr;
    unboundmp::models::Item* to_item = nullptr;
    
    for (auto& item : inv.items) {
        if (item.slot_index == from_slot && item.quantity > 0) from_item = &item;
        if (item.slot_index == to_slot && item.quantity > 0) to_item = &item;
    }

    if (!from_item || from_item->quantity < amount) return false;

    if (!to_item) {
        // Simple move or split to empty slot
        if (amount == from_item->quantity) {
            from_item->slot_index = to_slot;
        } else {
            // Split
            unboundmp::models::Item new_item = *from_item;
            new_item.slot_index = to_slot;
            new_item.quantity = amount;
            from_item->quantity -= amount;
            inv.items.push_back(new_item);
            inv.occupied_slots++;
        }
    } else {
        // Swap or Merge
        if (from_item->id == to_item->id && from_item->quantity > 0) {
            // Merge
            uint32_t space = to_item->stack_size - to_item->quantity;
            if (space >= amount) {
                to_item->quantity += amount;
                from_item->quantity -= amount;
                if (from_item->quantity == 0) inv.occupied_slots--;
            } else {
                return false; // Not enough space to merge the specified amount
            }
        } else {
            // Swap (only valid if amount == full stack)
            if (amount != from_item->quantity) return false;
            
            uint32_t temp = from_item->slot_index;
            from_item->slot_index = to_item->slot_index;
            to_item->slot_index = temp;
        }
    }
    
    inv.version_number++;
    inv.is_dirty = true;
    return true;
}

bool InventoryManager::AddItem(uint64_t character_id, const unboundmp::models::Item& item) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it == cache_.end()) return false;
    
    auto& inv = it->second;
    uint32_t remaining = item.quantity;
    
    // Try to merge with existing stacks first
    for (auto& existing : inv.items) {
        if (existing.id == item.id && existing.quantity < existing.stack_size && existing.quantity > 0) {
            uint32_t space = existing.stack_size - existing.quantity;
            uint32_t to_add = std::min(space, remaining);
            existing.quantity += to_add;
            remaining -= to_add;
            if (remaining == 0) break;
        }
    }
    
    // Add to new slots
    while (remaining > 0 && inv.occupied_slots < inv.max_slots) {
        // Find empty slot
        uint32_t free_slot = inv.max_slots;
        for (uint32_t i = 0; i < inv.max_slots; i++) {
            bool used = false;
            for (const auto& existing : inv.items) {
                if (existing.slot_index == i && existing.quantity > 0) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                free_slot = i;
                break;
            }
        }
        
        if (free_slot == inv.max_slots) break; // Should not happen due to occupied_slots check
        
        uint32_t to_add = std::min(item.stack_size, remaining);
        
        unboundmp::models::Item new_item = item;
        new_item.quantity = to_add;
        new_item.slot_index = free_slot;
        inv.items.push_back(new_item);
        inv.occupied_slots++;
        
        remaining -= to_add;
    }
    
    if (remaining < item.quantity) {
        inv.version_number++;
        inv.is_dirty = true;
    }
    
    return remaining == 0;
}

bool InventoryManager::RemoveItem(uint64_t character_id, uint32_t slot_index, uint32_t amount) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it == cache_.end()) return false;
    
    auto& inv = it->second;
    
    for (auto& item : inv.items) {
        if (item.slot_index == slot_index && item.quantity >= amount) {
            item.quantity -= amount;
            if (item.quantity == 0) inv.occupied_slots--;
            inv.version_number++;
            inv.is_dirty = true;
            return true;
        }
    }
    return false;
}

bool InventoryManager::AddMoney(uint64_t character_id, uint64_t amount) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it == cache_.end()) return false;
    
    it->second.money += amount;
    it->second.version_number++;
    it->second.is_dirty = true;
    return true;
}

bool InventoryManager::RemoveMoney(uint64_t character_id, uint64_t amount) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it == cache_.end()) return false;
    
    if (it->second.money < amount) return false;
    
    it->second.money -= amount;
    it->second.version_number++;
    it->second.is_dirty = true;
    return true;
}

bool InventoryManager::IsDirty(uint64_t character_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    return it != cache_.end() && it->second.is_dirty;
}

void InventoryManager::ClearDirty(uint64_t character_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_.find(character_id);
    if (it != cache_.end()) {
        it->second.is_dirty = false;
    }
}

} // namespace unboundmp::server::gameplay
