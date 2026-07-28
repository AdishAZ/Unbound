#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"

namespace unboundmp::memory {

class PcWriter {
 public:
  PcWriter(MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  bool Write(const std::vector<uint8_t>& blob) {
    const auto base_symbol = addresses_.Get("pc_base");
    if (!base_symbol) {
      return false;
    }
    
    // Safety check: ensure we don't write zero bytes or excessively large blobs
    if (blob.empty() || blob.size() > 100000) {
        return false;
    }

    memory_.WriteBytes(base_symbol->address, blob);
    return true;
  }

 private:
  MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
