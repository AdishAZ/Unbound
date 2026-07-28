#pragma once

#include <string>

#include "emulator/emulator_core.h"

namespace unboundmp::emulator {

// Validates and loads a save file into an already-ROM-loaded IEmulatorCore.
// Kept separate from RomLoader because save handling has its own rules
// (may not exist yet - that's fine, a fresh save is created - and the path
// is independently derivable from the ROM path).
class SaveLoader {
 public:
  // Given a ROM path like ".../pokemon_unbound.gba", returns the
  // conventional sibling save path ".../pokemon_unbound.sav". Purely a
  // string transform; does not touch the filesystem.
  static std::string DefaultSavePathFor(const std::string& rom_path);

  // Checks that save_path's parent directory exists and is writable
  // (creating a save there is fine even if the file itself doesn't exist
  // yet - a missing save file is not an error, an unwritable directory is).
  static EmulatorResult ValidatePath(const std::string& save_path);

  // Runs ValidatePath(), then IEmulatorCore::LoadSave(). `core` must
  // already have a ROM loaded (see IEmulatorCore::LoadSave() precondition).
  static EmulatorResult Load(IEmulatorCore& core, const std::string& save_path);

  // Convenience: Load(core, DefaultSavePathFor(rom_path)).
  static EmulatorResult LoadDefault(IEmulatorCore& core, const std::string& rom_path);
};

}  // namespace unboundmp::emulator
