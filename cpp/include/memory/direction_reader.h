#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

enum class FacingDirection {
  kUnknown,
  kSouth,
  kNorth,
  kWest,
  kEast,
};

// Maps a raw byte value from RAM to a FacingDirection. This is a *table*,
// not a hardcoded switch, because the raw encoding is not confirmed for
// Unbound's specific build - it's populated with a documented starting
// hypothesis (see kPokeemeraldDecompHypothesis below) that must be
// verified against the real game before being trusted. Load a verified
// mapping via LoadHypothesis()/Set() rather than assuming the default is
// correct.
class DirectionEncoding {
 public:
  void Set(uint32_t raw_value, FacingDirection direction) { map_[raw_value] = direction; }

  FacingDirection Decode(uint32_t raw_value) const {
    auto it = map_.find(raw_value);
    if (it == map_.end()) {
      return FacingDirection::kUnknown;
    }
    return it->second;
  }

  // Community disassembly projects for the vanilla engine Unbound is built
  // on (pret/pokeemerald) document the object-event "facingDirection" byte
  // as DIR_NONE=0, DIR_SOUTH=1, DIR_NORTH=2, DIR_WEST=3, DIR_EAST=4 (plus
  // diagonal/underwater variants 5-8 not modeled here). This is public
  // information about the *vanilla* engine's data encoding, sourced from
  // the pokeemerald decompilation project - it is NOT a claim about where
  // in Unbound's RAM this byte lives (that's address_table.h's job), and
  // it is NOT guaranteed Unbound kept this exact encoding. Treat this as a
  // starting hypothesis to verify (e.g. face each direction in-game and
  // confirm the configured address reads the expected value), not as a
  // verified fact.
  static DirectionEncoding PokeemeraldDecompHypothesis() {
    DirectionEncoding enc;
    enc.Set(1, FacingDirection::kSouth);
    enc.Set(2, FacingDirection::kNorth);
    enc.Set(3, FacingDirection::kWest);
    enc.Set(4, FacingDirection::kEast);
    return enc;
  }

 private:
  std::unordered_map<uint32_t, FacingDirection> map_;
};

class DirectionReader {
 public:
  DirectionReader(const MemoryApi& memory, const AddressTable& addresses, DirectionEncoding encoding)
      : memory_(memory), addresses_(addresses), encoding_(std::move(encoding)) {}

  ReadResult<FacingDirection> Read() const {
    const auto symbol = addresses_.Get("player_facing");
    if (!symbol) {
      return ReadResult<FacingDirection>::NotConfigured("player_facing");
    }
    const uint32_t raw = memory_.ReadWidth(symbol->address, symbol->width);
    return ReadResult<FacingDirection>::Success(encoding_.Decode(raw));
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
  DirectionEncoding encoding_;
};

}  // namespace unboundmp::memory
