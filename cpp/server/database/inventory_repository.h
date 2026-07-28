#pragma once

#include "database/database.h"
#include "models/inventory.h"
#include <memory>
#include <optional>

namespace unboundmp::server::database {

class InventoryRepository {
 public:
  explicit InventoryRepository(std::shared_ptr<DatabasePool> db_pool);

  // Loads the inventory for a character. Returns an empty but initialized 
  // Inventory if no blob exists.
  unboundmp::models::Inventory LoadInventory(uint64_t character_id);

  // Saves the full inventory state to the character's inventory blob.
  bool SaveInventory(uint64_t character_id, const unboundmp::models::Inventory& inventory);

  // Updates the money amount for a character in the characters table.
  bool UpdateMoney(uint64_t character_id, uint64_t money);

  // Loads the money amount from the characters table.
  uint64_t LoadMoney(uint64_t character_id);

 private:
  std::shared_ptr<DatabasePool> db_pool_;
};

}  // namespace unboundmp::server::database
