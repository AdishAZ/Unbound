#pragma once

#include <cstdint>
#include <string>

namespace unboundmp::models {

enum class ItemType : uint8_t {
    kGeneral = 0,
    kMedicine = 1,
    kPokeball = 2,
    kKeyItem = 3,
    kBerry = 4,
    kTMHM = 5,
    kBattleItem = 6,
    kMail = 7
};

struct Item {
    uint32_t id = 0;
    ItemType type = ItemType::kGeneral;
    std::string name;
    uint32_t quantity = 0;
    uint32_t slot_index = 0;
    uint32_t stack_size = 99; // Maximum quantity allowed in this stack
    
    // Future expansion fields
    uint64_t flags = 0; 
    std::string metadata;
    int64_t created_at = 0;
};

} // namespace unboundmp::models
