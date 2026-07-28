#pragma once

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

// Map identity, using the (bank, number) convention common to GBA-era
// Pokemon engines (pret/pokeemerald calls these mapGroup/mapNum) rather
// than a single global map id.
struct MapId {
  uint32_t bank = 0;
  uint32_t number = 0;

  bool operator==(const MapId& other) const {
    return bank == other.bank && number == other.number;
  }
};

// Reads the player's current map from whatever addresses the AddressTable
// has configured under "player_map_bank" / "player_map_number". Neither
// address is known ahead of time - see docs/REVERSE_ENGINEERING.md.
class MapReader {
 public:
  MapReader(const MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  ReadResult<MapId> Read() const {
    const auto sb1_symbol = addresses_.Get("save_block_1_ptr");
    if (!sb1_symbol) {
      return ReadResult<MapId>::NotConfigured("save_block_1_ptr");
    }

    uint32_t sb1 = memory_.ReadWidth(sb1_symbol->address, sb1_symbol->width);
    if (sb1 == 0) return ReadResult<MapId>::Failure("save_block_1_ptr is null");

    MapId id;
    id.bank = memory_.ReadWidth(sb1 + 4, ValueWidth::kU8);
    id.number = memory_.ReadWidth(sb1 + 5, ValueWidth::kU8);
    return ReadResult<MapId>::Success(id);
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
