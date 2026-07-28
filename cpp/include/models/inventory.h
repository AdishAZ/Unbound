#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include "models/item.h"

namespace unboundmp::models {

struct Inventory {
    uint32_t max_slots = 30; // Configurable max capacity
    uint32_t occupied_slots = 0;
    
    // Conceptually money is part of the inventory for UI purposes,
    // though the DB may store it directly on the Character row.
    uint64_t money = 0; 
    
    uint64_t version_number = 0; // Incremented on every mutation for strict ordering
    bool is_dirty = false;
    
    std::vector<Item> items;
    
    // Helper methods
    std::optional<Item> GetItemAt(uint32_t slot_index) const {
        for (const auto& item : items) {
            if (item.slot_index == slot_index && item.quantity > 0) {
                return item;
            }
        }
        return std::nullopt;
    }
};

} // namespace unboundmp::models
