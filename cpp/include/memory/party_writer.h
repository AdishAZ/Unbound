#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/party_reader.h" // For RawPartySlot and PartyLayout

namespace unboundmp::memory {

class PartyWriter {
 public:
  PartyWriter(MemoryApi& memory, const AddressTable& addresses, PartyLayout layout = {})
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  bool Write(const std::vector<RawPartySlot>& slots) {
    const auto count_symbol = addresses_.Get("party_count");
    if (!count_symbol) return false;
    
    const auto base_symbol = addresses_.Get("party_base");
    if (!base_symbol) return false;

    uint32_t count = static_cast<uint32_t>(slots.size());
    if (count > layout_.max_party_size) {
        count = layout_.max_party_size;
    }

    // Write party count
    memory_.WriteWidth(count_symbol->address, count_symbol->width, count);

    // Write slots
    for (uint32_t i = 0; i < count; ++i) {
      const uint32_t slot_address = base_symbol->address + i * layout_.slot_stride_bytes;
      
      // Ensure we don't write more than slot stride
      size_t write_len = std::min(static_cast<size_t>(layout_.slot_stride_bytes), slots[i].bytes.size());
      
      std::vector<uint8_t> buffer = slots[i].bytes;
      if (buffer.size() > write_len) {
          buffer.resize(write_len);
      }
      
      memory_.WriteBytes(slot_address, buffer);
    }
    
    return true;
  }

 private:
  MemoryApi& memory_;
  const AddressTable& addresses_;
  PartyLayout layout_;
};

}  // namespace unboundmp::memory
