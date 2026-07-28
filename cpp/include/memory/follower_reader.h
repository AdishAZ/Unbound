#pragma once

#include <cstdint>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

// State of the following first-party Pokemon (Unbound's overworld
// follower feature). species_id == 0 is treated as "no follower", matching
// the FollowerUpdate.species_id convention already established in
// proto/packets.proto (species_id = 0 = no follower / follower hidden) -
// kept consistent here so a future milestone wiring this reader up to that
// packet doesn't need a translation step.
struct FollowerState {
  uint32_t species_id = 0;
  bool visible = false;
  bool shiny = false;
};

class FollowerReader {
 public:
  FollowerReader(const MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  ReadResult<FollowerState> Read() const {
    const auto species_symbol = addresses_.Get("follower_species");
    if (!species_symbol) {
      return ReadResult<FollowerState>::NotConfigured("follower_species");
    }

    FollowerState state;
    state.species_id = memory_.ReadWidth(species_symbol->address, species_symbol->width);

    // visible/shiny are optional on top of species - a lot of value can be
    // had (e.g. showing the follower at all) with just species configured,
    // so these don't block the whole read the way species does.
    if (const auto visible_symbol = addresses_.Get("follower_visible")) {
      state.visible = memory_.ReadWidth(visible_symbol->address, visible_symbol->width) != 0;
    } else {
      state.visible = state.species_id != 0;
    }

    if (const auto shiny_symbol = addresses_.Get("follower_shiny")) {
      state.shiny = memory_.ReadWidth(shiny_symbol->address, shiny_symbol->width) != 0;
    }

    return ReadResult<FollowerState>::Success(state);
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
