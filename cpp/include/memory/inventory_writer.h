#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"

namespace unboundmp::memory {

class InventoryWriter {
 public:
  InventoryWriter(MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  bool Write(const std::vector<uint8_t>& blob) {
    const auto base_ptr_symbol = addresses_.Get("save_block_1_ptr");
    if (!base_ptr_symbol) {
      return false;
    }
    
    uint32_t save_block_1_addr = memory_.ReadU32(base_ptr_symbol->address);
    if (save_block_1_addr == 0) {
        return false;
    }
    uint32_t inventory_address = save_block_1_addr + 0x0290;
    
    // Safety check: ensure we don't write zero bytes or excessively large blobs
    if (blob.empty() || blob.size() > 65536) {
        return false;
    }

    memory_.WriteBytes(inventory_address, blob);
    return true;
  }

 private:
  MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
