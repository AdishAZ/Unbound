#include "database/inventory_repository.h"
#include "network/serializer.h"
#include <pqxx/pqxx>
#include <iostream>

namespace unboundmp::server::database {

InventoryRepository::InventoryRepository(std::shared_ptr<DatabasePool> db_pool)
    : db_pool_(std::move(db_pool)) {}

unboundmp::models::Inventory InventoryRepository::LoadInventory(uint64_t character_id) {
    unboundmp::models::Inventory inventory;
    inventory.money = LoadMoney(character_id); // Fetch money from characters table

    try {
        db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::row r = txn.exec_params1(
                "SELECT blob_data FROM inventory_blobs WHERE character_id = $1", 
                character_id
            );
            auto field = r[0];
            if (field.is_null()) {
                inventory.max_slots = 60;
                inventory.occupied_slots = 0;
                inventory.version_number = 1;
                inventory.is_dirty = false;
                return true;
            }
            auto bin_data = field.as<std::basic_string<std::byte>>();
            std::vector<uint8_t> data(reinterpret_cast<const uint8_t*>(bin_data.data()), reinterpret_cast<const uint8_t*>(bin_data.data()) + bin_data.size());
            if (data.empty()) {
                inventory.max_slots = 60;
                inventory.occupied_slots = 0;
                inventory.version_number = 1;
                inventory.is_dirty = false;
                return true;
            }
            unboundmp::network::Serializer s(data);
            
            inventory.max_slots = s.ReadU32();
            inventory.occupied_slots = s.ReadU32();
            // Money is skipped from the blob because it's stored in characters table
            uint64_t _blob_money = s.ReadU64(); 
            inventory.version_number = s.ReadU64();
            inventory.is_dirty = false;
            
            uint32_t num_items = s.ReadU32();
            for (uint32_t i = 0; i < num_items; i++) {
                unboundmp::models::Item item;
                item.id = s.ReadU32();
                item.type = static_cast<unboundmp::models::ItemType>(s.ReadU8());
                item.name = s.ReadString();
                item.quantity = s.ReadU32();
                item.slot_index = s.ReadU32();
                item.stack_size = s.ReadU32();
                item.flags = s.ReadU64();
                item.metadata = s.ReadString();
                item.created_at = static_cast<int64_t>(s.ReadU64());
                inventory.items.push_back(item);
            }
            return true;
        });
    } catch (const pqxx::unexpected_rows&) {
        // No row found, returns an empty inventory
    } catch (const std::exception& e) {
        std::cerr << "Failed to load inventory for character " << character_id << ": " << e.what() << std::endl;
    }
    
    return inventory;
}

bool InventoryRepository::SaveInventory(uint64_t character_id, const unboundmp::models::Inventory& inventory) {
    // First, sync money to the characters table
    UpdateMoney(character_id, inventory.money);

    unboundmp::network::Serializer s;
    s.WriteU32(inventory.max_slots);
    s.WriteU32(inventory.occupied_slots);
    s.WriteU64(inventory.money);
    s.WriteU64(inventory.version_number);
    s.WriteU32(static_cast<uint32_t>(inventory.items.size()));
    for (const auto& item : inventory.items) {
        s.WriteU32(item.id);
        s.WriteU8(static_cast<uint8_t>(item.type));
        s.WriteString(item.name);
        s.WriteU32(item.quantity);
        s.WriteU32(item.slot_index);
        s.WriteU32(item.stack_size);
        s.WriteU64(item.flags);
        s.WriteString(item.metadata);
        s.WriteU64(static_cast<uint64_t>(item.created_at));
    }

    try {
        const auto& buffer = s.GetBuffer();
        db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            std::basic_string_view<std::byte> bin_data(reinterpret_cast<const std::byte*>(buffer.data()), buffer.size());
            txn.exec_params(
                R"(
                INSERT INTO inventory_blobs (character_id, blob_data) 
                VALUES ($1, $2)
                ON CONFLICT (character_id) DO UPDATE SET blob_data = EXCLUDED.blob_data
                )",
                character_id, bin_data
            );
            return true;
        });
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save inventory for character " << character_id << ": " << e.what() << std::endl;
        return false;
    }
}

bool InventoryRepository::UpdateMoney(uint64_t character_id, uint64_t money) {
    try {
        db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            txn.exec_params(
                "UPDATE characters SET money = $1 WHERE id = $2",
                money, character_id
            );
            return true;
        });
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to update money for character " << character_id << ": " << e.what() << std::endl;
        return false;
    }
}

uint64_t InventoryRepository::LoadMoney(uint64_t character_id) {
    uint64_t money = 0;
    try {
        db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::row r = txn.exec_params1(
                "SELECT money FROM characters WHERE id = $1", 
                character_id
            );
            money = r[0].as<uint64_t>();
            return true;
        });
    } catch (const pqxx::unexpected_rows&) {
        // Character not found, money is 0
    } catch (const std::exception& e) {
        std::cerr << "Failed to load money for character " << character_id << ": " << e.what() << std::endl;
    }
    return money;
}

}  // namespace unboundmp::server::database
