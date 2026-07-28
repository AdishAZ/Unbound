#include "emulator/save_loader.h"

#include <filesystem>
#include <fstream>

namespace unboundmp::emulator {

namespace fs = std::filesystem;

std::string SaveLoader::DefaultSavePathFor(const std::string& rom_path) {
  fs::path path(rom_path);
  path.replace_extension(".sav");
  return path.string();
}

EmulatorResult SaveLoader::ValidatePath(const std::string& save_path) {
  if (save_path.empty()) {
    return EmulatorResult::Failure("Save path is empty");
  }

  fs::path path(save_path);
  fs::path parent = path.has_parent_path() ? path.parent_path() : fs::path(".");

  std::error_code ec;
  if (!fs::exists(parent, ec) || !fs::is_directory(parent, ec)) {
    return EmulatorResult::Failure("Save directory does not exist: " + parent.string());
  }

  // Probe writability without leaving a stray file behind: if save_path
  // itself already exists, check it's an existing regular file (we'll
  // overwrite in place); otherwise try creating+removing a temp probe file
  // in the same directory.
  if (fs::exists(path, ec)) {
    if (!fs::is_regular_file(path, ec)) {
      return EmulatorResult::Failure("Save path exists but is not a regular file: " + save_path);
    }
    std::ofstream test(save_path, std::ios::binary | std::ios::app);
    if (!test.is_open()) {
      return EmulatorResult::Failure("Save file is not writable: " + save_path);
    }
    return EmulatorResult::Success();
  }

  const fs::path probe = parent / ".unboundmp_write_probe";
  std::ofstream probe_file(probe, std::ios::binary);
  if (!probe_file.is_open()) {
    return EmulatorResult::Failure("Save directory is not writable: " + parent.string());
  }
  probe_file.close();
  fs::remove(probe, ec);

  return EmulatorResult::Success();
}

EmulatorResult SaveLoader::Load(IEmulatorCore& core, const std::string& save_path) {
  const EmulatorResult validation = ValidatePath(save_path);
  if (!validation) {
    return validation;
  }
  return core.LoadSave(save_path);
}

EmulatorResult SaveLoader::LoadDefault(IEmulatorCore& core, const std::string& rom_path) {
  return Load(core, DefaultSavePathFor(rom_path));
}

}  // namespace unboundmp::emulator
