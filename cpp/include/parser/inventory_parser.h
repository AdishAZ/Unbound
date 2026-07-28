#pragma once

#include "memory/inventory_reader.h"
#include "parser/parse_result.h"
#include "models/inventory.h"

namespace unboundmp::parser {

class InventoryParser {
 public:
  InventoryParser(const memory::MemoryApi& memory, const memory::AddressTable& addresses);

  ParseResult<models::Inventory> Parse(int64_t frame_count);

 private:
  memory::InventoryReader reader_;
};

} // namespace unboundmp::parser
