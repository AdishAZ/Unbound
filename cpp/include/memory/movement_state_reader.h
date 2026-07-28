#pragma once

#include <cstdint>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

// Covers the "Running state / Surf state / Bike state" deliverables
// together, since in the vanilla engine Unbound is built on these are all
// bits of one flags field (pret/pokeemerald's `gPlayerAvatar.flags`,
// PLAYER_AVATAR_FLAG_* constants), not separate fields. If reverse
// engineering finds Unbound stores these differently (e.g. separate
// booleans), MovementBitLayout below can be reconfigured per-bit instead
// of assuming the vanilla layout, without changing this reader's shape.
struct MovementState {
  bool on_foot = false;
  bool running = false;
  bool on_mach_bike = false;
  bool on_acro_bike = false;
  bool surfing = false;
  bool underwater = false;

  bool OnBike() const { return on_mach_bike || on_acro_bike; }
};

// Bit positions within the configured "player_movement_flags" symbol.
// Defaults below are the pokeemerald decomp's PLAYER_AVATAR_FLAG_* bit
// indices - a documented hypothesis about the *vanilla* engine's bit
// layout, not a verified fact about Unbound's RAM contents or even that
// Unbound uses a single flags byte at all. Verify before trusting (e.g.
// start surfing and confirm the configured bit flips) - see
// docs/REVERSE_ENGINEERING.md.
struct MovementBitLayout {
  uint8_t on_foot_bit = 0;
  uint8_t on_mach_bike_bit = 1;
  uint8_t on_acro_bike_bit = 2;
  uint8_t surfing_bit = 3;
  uint8_t underwater_bit = 4;
  // Running is not a flags-field bit in the vanilla decomp (it's derived
  // from held B-button + on_foot), so it's tracked as a separate optional
  // symbol ("player_running") rather than a bit here. If RE finds Unbound
  // does store it as a flag bit, add running_bit here instead and update
  // MovementStateReader::Read() accordingly.

  static MovementBitLayout PokeemeraldDecompHypothesis() { return MovementBitLayout{}; }
};

class MovementStateReader {
 public:
  MovementStateReader(const MemoryApi& memory, const AddressTable& addresses,
                       MovementBitLayout layout)
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  ReadResult<MovementState> Read() const {
    const auto flags_symbol = addresses_.Get("player_movement_flags");
    if (!flags_symbol) {
      return ReadResult<MovementState>::NotConfigured("player_movement_flags");
    }

    const uint32_t flags = memory_.ReadWidth(flags_symbol->address, flags_symbol->width);

    MovementState state;
    state.on_mach_bike = BitSet(flags, layout_.on_mach_bike_bit);
    state.on_acro_bike = BitSet(flags, layout_.on_acro_bike_bit);
    state.surfing = BitSet(flags, layout_.surfing_bit);
    state.underwater = BitSet(flags, layout_.underwater_bit);
    state.on_foot = !state.OnBike() && !state.surfing && !state.underwater;

    // Running has its own optional symbol - not every AddressTable will
    // have it configured even if player_movement_flags is, so its absence
    // doesn't fail the whole read; it just leaves `running` false.
    if (const auto running_symbol = addresses_.Get("player_running")) {
      state.running = memory_.ReadWidth(running_symbol->address, running_symbol->width) != 0;
    }

    return ReadResult<MovementState>::Success(state);
  }

 private:
  static bool BitSet(uint32_t value, uint8_t bit) { return ((value >> bit) & 0x1) != 0; }

  const MemoryApi& memory_;
  const AddressTable& addresses_;
  MovementBitLayout layout_;
};

}  // namespace unboundmp::memory
