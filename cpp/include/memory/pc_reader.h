#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

struct PcLayout {
  uint32_t size_bytes = 0;
};

class PcReader {
 public:
  PcReader(const MemoryApi& memory, const AddressTable& addresses, PcLayout layout = {})
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  ReadResult<std::vector<uint8_t>> Read() const {
    const auto base_symbol = addresses_.Get("pc_base");
    if (!base_symbol) {
      return ReadResult<std::vector<uint8_t>>::NotConfigured("pc_base");
    }
    const auto size_symbol = addresses_.Get("pc_size");
    
    uint32_t size = layout_.size_bytes;
    if (size_symbol) {
        size = memory_.ReadWidth(size_symbol->address, size_symbol->width);
    }
    
    if (size == 0) {
        // Fallback: 14 boxes * 30 pokemon * 80 bytes = 33600 bytes
        // Emerald/FireRed usually uses exactly 33600 bytes (834*40 bytes actually is PC?).
        // Let's use a generous fallback.
        size = 33600;
    }

    std::vector<uint8_t> bytes = memory_.ReadBytes(base_symbol->address, size);
    return ReadResult<std::vector<uint8_t>>::Success(std::move(bytes));
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
  PcLayout layout_;
};

}  // namespace unboundmp::memory
