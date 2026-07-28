#pragma once

#include <string>

#include "emulator/emulator_core.h"

namespace unboundmp::emulator {

// Read-only ROM header info, parsed from the file on disk without ever
// opening it for writing. Field sizes/offsets are the real GBA cartridge
// header layout (GBATek "GBA Cartridge Header"):
//   0x0A0..0x0AB  game title   (12 bytes, space-padded, may be shorter)
//   0x0AC..0x0AF  game code    (4 bytes)
//   0x0B0..0x0B1  maker code   (2 bytes)
struct RomHeaderInfo {
  std::string game_title;
  std::string game_code;
  std::string maker_code;
};

// Validates and loads a ROM file into an already-Initialize()'d
// IEmulatorCore. This exists as its own small component (rather than just
// calling core.LoadRom() directly) so callers get:
//   - a clear pre-flight error before ever touching the emulator core
//     (missing file, empty file, wrong extension)
//   - a read-only header sanity check they can show the player /
//     log ("this looks like <title>/<code>") before committing to loading
// It never writes to the ROM file, never patches it, and never modifies it
// on disk or in memory - `ReadHeader()` opens the file with std::ifstream
// in read-only binary mode and nothing else.
class RomLoader {
 public:
  // Checks the file exists, is non-empty, and has a plausible GBA ROM
  // extension (.gba, case-insensitive). Does not open the emulator core.
  static EmulatorResult ValidatePath(const std::string& rom_path);

  // Reads just the cartridge header fields (see RomHeaderInfo) directly
  // from the file via a read-only ifstream - does not touch the emulator
  // core at all. Useful for showing "here's what this file claims to be"
  // before deciding to load it.
  static EmulatorResult ReadHeader(const std::string& rom_path, RomHeaderInfo& out_info);

  // Runs ValidatePath(), then IEmulatorCore::LoadRom(). `core` must already
  // have had Initialize() called successfully.
  static EmulatorResult Load(IEmulatorCore& core, const std::string& rom_path);
};

}  // namespace unboundmp::emulator
