#pragma once

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

// Player position in map-relative tile coordinates. Whether this is the
// overworld "current position" or something engine-specific like a
// "position including in-flight movement offset" depends entirely on which
// RAM address ends up configured for player_x/player_y - see
// docs/REVERSE_ENGINEERING.md for how to tell the difference while probing
// (walk one tile and see whether the value changes instantly on keypress
// or animates smoothly across several frames).
struct Position {
  int32_t x = 0;
  int32_t y = 0;
};

class PositionReader {
 public:
  PositionReader(const MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  ReadResult<Position> Read() const {
    const auto x_symbol = addresses_.Get("player_x");
    if (!x_symbol) {
      return ReadResult<Position>::NotConfigured("player_x");
    }
    const auto y_symbol = addresses_.Get("player_y");
    if (!y_symbol) {
      return ReadResult<Position>::NotConfigured("player_y");
    }

    Position pos;
    pos.x = x_symbol->is_signed ? memory_.ReadWidthSigned(x_symbol->address, x_symbol->width)
                                 : static_cast<int32_t>(memory_.ReadWidth(x_symbol->address, x_symbol->width));
    pos.y = y_symbol->is_signed ? memory_.ReadWidthSigned(y_symbol->address, y_symbol->width)
                                 : static_cast<int32_t>(memory_.ReadWidth(y_symbol->address, y_symbol->width));
    return ReadResult<Position>::Success(pos);
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
