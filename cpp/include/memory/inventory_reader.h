#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

struct InventoryLayout {
  uint32_t size_bytes = 0;
};

class InventoryReader {
 public:
  InventoryReader(const MemoryApi& memory, const AddressTable& addresses, InventoryLayout layout = {})
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  ReadResult<std::vector<uint8_t>> Read() const {
    const auto base_ptr_symbol = addresses_.Get("save_block_1_ptr");
    if (!base_ptr_symbol) {
      return ReadResult<std::vector<uint8_t>>::NotConfigured("save_block_1_ptr");
    }
    
    uint32_t save_block_1_addr = memory_.ReadU32(base_ptr_symbol->address);
    if (save_block_1_addr == 0) {
        return ReadResult<std::vector<uint8_t>>::Failure("save block 1 ptr is null");
    }
    uint32_t inventory_address = save_block_1_addr + 0x0290;
    
    const auto size_symbol = addresses_.Get("inventory_size");
    
    uint32_t size = layout_.size_bytes;
    if (size_symbol) {
        size = memory_.ReadWidth(size_symbol->address, size_symbol->width);
    }
    
    if (size == 0) {
        // Fallback to a generous maximum if unconfigured
        size = 4096;
    }

    std::vector<uint8_t> bytes = memory_.ReadBytes(inventory_address, size);
    return ReadResult<std::vector<uint8_t>>::Success(std::move(bytes));
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
  InventoryLayout layout_;
};

}  // namespace unboundmp::memory
