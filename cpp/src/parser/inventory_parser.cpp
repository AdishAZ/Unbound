#include "parser/inventory_parser.h"
#include <fstream>
#include <iostream>
#include <string>

namespace unboundmp::parser {

InventoryParser::InventoryParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses)
    : reader_(memory, addresses) {}

ParseResult<models::Inventory> InventoryParser::Parse(int64_t frame_count) {
    auto raw_res = reader_.Read();
    if (!raw_res.ok() || !raw_res.value.has_value()) {
        std::ofstream out("inventory_debug.txt", std::ios::app);
        out << "[InventoryParser] Read failed! Error: " << raw_res.error << std::endl;
        return ParseResult<models::Inventory>::Failure(raw_res.error);
    }

    const auto& bytes = *raw_res.value;
    if (bytes.empty() || bytes.size() % 4 != 0) {
        std::ofstream out("inventory_debug.txt", std::ios::app);
        out << "[InventoryParser] Read failed! Invalid byte size: " << bytes.size() << std::endl;
        return ParseResult<models::Inventory>::Failure("Invalid inventory byte size");
    }

    models::Inventory inventory;
    
    // DEBUG LOGGING
    int non_zero_count = 0;
    for (size_t i = 0; i < bytes.size() - 3; i += 4) {
        if (bytes[i] != 0 || bytes[i+1] != 0) non_zero_count++;
    }
    if (frame_count % 60 == 0) {
        std::ofstream out("inventory_debug.txt", std::ios::app);
        out << "[InventoryParser] Read " << bytes.size() << " bytes. Non-zero slots: " << non_zero_count << std::endl;
    }
    
    // FireRed Standard Pocket Layout (in item slots):
    // Items: 30
    // Key Items: 30
    // Poke Balls: 13
    // TMs/HMs: 58
    // Berries: 43
    // Total slots: 174
    
    const uint32_t ITEMS_START = 0;
    const uint32_t KEY_ITEMS_START = 30;
    const uint32_t POKEBALLS_START = 60;
    const uint32_t TM_HM_START = 73;
    const uint32_t BERRIES_START = 131;
    const uint32_t TOTAL_SLOTS = 174;

    inventory.max_slots = TOTAL_SLOTS;
    inventory.occupied_slots = 0;
    
    // First 4 bytes are money (encrypted, but we parse it raw for now)
    // Next 4 bytes are coins/registered items.
    // Real items start at offset 8.
    if (bytes.size() >= 8) {
        inventory.money = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    } else {
        inventory.money = 0;
    }

    for (size_t i = 8; i < bytes.size() && (i - 8) / 4 < TOTAL_SLOTS; i += 4) {
        uint16_t item_id = bytes[i] | (bytes[i + 1] << 8);
        uint16_t quantity = bytes[i + 2] | (bytes[i + 3] << 8);

        if (item_id != 0 && quantity > 0) {
            models::Item item;
            item.id = item_id;
            item.quantity = quantity;
            item.slot_index = static_cast<uint32_t>((i - 8) / 4);
            
            // Assign name (placeholder based on ID, in reality would use a ROM item data table)
            item.name = "Item #" + std::to_string(item_id);
            
            // Assign type based on pocket
            if (item.slot_index >= ITEMS_START && item.slot_index < KEY_ITEMS_START) {
                item.type = models::ItemType::kGeneral;
            } else if (item.slot_index >= KEY_ITEMS_START && item.slot_index < POKEBALLS_START) {
                item.type = models::ItemType::kKeyItem;
            } else if (item.slot_index >= POKEBALLS_START && item.slot_index < TM_HM_START) {
                item.type = models::ItemType::kPokeball;
            } else if (item.slot_index >= TM_HM_START && item.slot_index < BERRIES_START) {
                item.type = models::ItemType::kTMHM;
            } else if (item.slot_index >= BERRIES_START) {
                item.type = models::ItemType::kBerry;
            }
            
            inventory.items.push_back(item);
            inventory.occupied_slots++;
        }
    }

    return ParseResult<models::Inventory>::Success(std::move(inventory));
}

} // namespace unboundmp::parser
