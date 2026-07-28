#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

// Layout constants for the party array that are *not* per-installation
// addresses (those go in AddressTable) but plain numeric facts about the
// struct shape. slot_stride_bytes defaults to 100, the size of
// `struct Pokemon` in the public pokeemerald decompilation (Generation III
// party Pokemon struct) - a real, citable fact about the vanilla engine's
// data layout, not a memory address. It's still only a hypothesis for
// Unbound specifically until confirmed (e.g. does party_count * stride
// land exactly on a sensible boundary; do known-good save-import tools
// agree), so it's overridable here rather than hardcoded into the reader.
struct PartyLayout {
  uint32_t slot_stride_bytes = 100;
  uint32_t max_party_size = 6;
};

// One party slot's completely raw, undecoded bytes. Generation III party
// Pokemon data is both order-shuffled and XOR-encrypted based on the
// Pokemon's personality value and OT ID (public knowledge about the
// vanilla game engine's save format) - decrypting it is real game-logic
// work explicitly out of scope for this milestone. PartyReader hands back
// raw bytes so a later milestone can add decryption without this reader
// needing to change.
struct RawPartySlot {
  std::vector<uint8_t> bytes;
};

class PartyReader {
 public:
  PartyReader(const MemoryApi& memory, const AddressTable& addresses, PartyLayout layout = {})
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  ReadResult<std::vector<RawPartySlot>> Read() const {
    const auto count_symbol = addresses_.Get("party_count");
    if (!count_symbol) {
      return ReadResult<std::vector<RawPartySlot>>::NotConfigured("party_count");
    }
    const auto base_symbol = addresses_.Get("party_base");
    if (!base_symbol) {
      return ReadResult<std::vector<RawPartySlot>>::NotConfigured("party_base");
    }

    uint32_t count = memory_.ReadWidth(count_symbol->address, count_symbol->width);
    if (count > layout_.max_party_size) {
      // A count larger than the game allows almost always means we're
      // reading the wrong address (misconfigured symbol, or the game
      // hasn't finished initializing party data yet) rather than a real
      // party size - clamp defensively instead of reading out of bounds.
      count = layout_.max_party_size;
    }

    std::vector<RawPartySlot> slots;
    slots.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
      const uint32_t slot_address = base_symbol->address + i * layout_.slot_stride_bytes;
      RawPartySlot slot;
      slot.bytes = memory_.ReadBytes(slot_address, layout_.slot_stride_bytes);
      slots.push_back(std::move(slot));
    }

    return ReadResult<std::vector<RawPartySlot>>::Success(std::move(slots));
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
  PartyLayout layout_;
};

}  // namespace unboundmp::memory
