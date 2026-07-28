#pragma once

#include <cstdint>
#include <vector>

#include "emulator/emulator_core.h"

namespace unboundmp::memory {

// Bit width for a generic memory value. Kept as an explicit enum (rather
// than just "pass a byte count") so callers/config files can be explicit
// and so MemoryApi::ReadWidth() has an exhaustive switch.
enum class ValueWidth : uint8_t {
  kU8 = 1,
  kU16 = 2,
  kU32 = 4,
};

// Thin, game-agnostic wrapper around IEmulatorCore's raw bus reads. This is
// the *only* place in the client that talks directly to
// IEmulatorCore::ReadU8/16/32/ReadBytes - every game-state reader
// (map_reader, position_reader, party_reader, etc.) goes through this
// class, which itself knows nothing about Pokemon, Unbound, or any
// particular struct layout. That knowledge lives in address_table.h
// (addresses) and the individual reader classes (interpretation).
class MemoryApi {
 public:
  explicit MemoryApi(emulator::IEmulatorCore& core) : core_(core) {}

  uint8_t ReadU8(uint32_t address) const { return core_.ReadU8(address); }
  uint16_t ReadU16(uint32_t address) const { return core_.ReadU16(address); }
  uint32_t ReadU32(uint32_t address) const { return core_.ReadU32(address); }

  // Reads `width` bytes at `address` and returns them zero-extended into a
  // uint32_t. Lets generic/config-driven readers (see address_table.h)
  // read a value without a compile-time-known width.
  uint32_t ReadWidth(uint32_t address, ValueWidth width) const {
    switch (width) {
      case ValueWidth::kU8:
        return ReadU8(address);
      case ValueWidth::kU16:
        return ReadU16(address);
      case ValueWidth::kU32:
        return ReadU32(address);
    }
    return 0;
  }

  // Same as ReadWidth, but sign-extends the result based on `width`. Use
  // for fields that are genuinely signed (e.g. some engines store position
  // deltas as signed values); most Pokemon-engine coordinate fields are
  // unsigned map-relative tile indices, but this is here for the fields
  // that aren't.
  int32_t ReadWidthSigned(uint32_t address, ValueWidth width) const {
    switch (width) {
      case ValueWidth::kU8:
        return static_cast<int32_t>(static_cast<int8_t>(ReadU8(address)));
      case ValueWidth::kU16:
        return static_cast<int32_t>(static_cast<int16_t>(ReadU16(address)));
      case ValueWidth::kU32:
        return static_cast<int32_t>(ReadU32(address));
    }
    return 0;
  }

  // Bulk-reads `length` bytes starting at `address`. Used by readers that
  // pull whole structs (e.g. party_reader's per-slot Pokemon bytes) rather
  // than individual fields.
  std::vector<uint8_t> ReadBytes(uint32_t address, size_t length) const {
    std::vector<uint8_t> out(length);
    core_.ReadBytes(address, out.data(), length);
    return out;
  }

  // --- Stage 8: Memory Writing ---
  void WriteU8(uint32_t address, uint8_t value) { core_.WriteU8(address, value); }
  void WriteU16(uint32_t address, uint16_t value) { core_.WriteU16(address, value); }
  void WriteU32(uint32_t address, uint32_t value) { core_.WriteU32(address, value); }

  void WriteWidth(uint32_t address, ValueWidth width, uint32_t value) {
    switch (width) {
      case ValueWidth::kU8: WriteU8(address, static_cast<uint8_t>(value)); break;
      case ValueWidth::kU16: WriteU16(address, static_cast<uint16_t>(value)); break;
      case ValueWidth::kU32: WriteU32(address, value); break;
    }
  }

  void WriteBytes(uint32_t address, const std::vector<uint8_t>& data) {
    core_.WriteBytes(address, data.data(), data.size());
  }

 private:
  emulator::IEmulatorCore& core_;
};

}  // namespace unboundmp::memory
