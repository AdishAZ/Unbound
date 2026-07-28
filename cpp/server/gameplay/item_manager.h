#pragma once

#include "gameplay/inventory_manager.h"
#include <memory>

namespace unboundmp::server::gameplay {

class ItemManager {
public:
    explicit ItemManager(std::shared_ptr<InventoryManager> inventory_manager);
    ~ItemManager() = default;

    // Validates if an item can be used on a target
    bool ValidateUse(uint64_t character_id, uint32_t slot_index, uint32_t target_entity);

    // Consumes an item and applies its effect
    bool ConsumeItem(uint64_t character_id, uint32_t slot_index, uint32_t target_entity);

private:
    std::shared_ptr<InventoryManager> inventory_manager_;
};

} // namespace unboundmp::server::gameplay
