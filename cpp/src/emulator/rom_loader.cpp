#include "emulator/rom_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace unboundmp::emulator {

namespace {

bool HasExtension(const std::string& path, const std::string& ext_lower) {
  if (path.size() < ext_lower.size()) {
    return false;
  }
  std::string tail = path.substr(path.size() - ext_lower.size());
  std::transform(tail.begin(), tail.end(), tail.begin(),
                  [](unsigned char c) { return std::tolower(c); });
  return tail == ext_lower;
}

// Reads a fixed-size, space-padded field from an already-open file at a
// given offset and trims trailing spaces/NULs. `file` is not modified.
std::string ReadFixedField(std::ifstream& file, std::streamoff offset, size_t size) {
  std::string buffer(size, '\0');
  file.seekg(offset, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(size));
  if (!file) {
    return "";
  }
  while (!buffer.empty() && (buffer.back() == ' ' || buffer.back() == '\0')) {
    buffer.pop_back();
  }
  return buffer;
}

}  // namespace

EmulatorResult RomLoader::ValidatePath(const std::string& rom_path) {
  if (rom_path.empty()) {
    return EmulatorResult::Failure("ROM path is empty");
  }
  if (!HasExtension(rom_path, ".gba")) {
    return EmulatorResult::Failure("ROM path does not end in .gba: " + rom_path);
  }

  std::ifstream file(rom_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return EmulatorResult::Failure("Could not open ROM file: " + rom_path);
  }
  const std::streamoff size = file.tellg();
  if (size <= 0) {
    return EmulatorResult::Failure("ROM file is empty: " + rom_path);
  }
  // 0x0B2 is the end of the fixed header fields this loader inspects
  // (see RomHeaderInfo); anything smaller cannot be a real GBA ROM.
  if (size < 0xC0) {
    return EmulatorResult::Failure("ROM file is too small to contain a valid GBA header: " +
                                    rom_path);
  }

  return EmulatorResult::Success();
}

EmulatorResult RomLoader::ReadHeader(const std::string& rom_path, RomHeaderInfo& out_info) {
  const EmulatorResult validation = ValidatePath(rom_path);
  if (!validation) {
    return validation;
  }

  std::ifstream file(rom_path, std::ios::binary);
  if (!file.is_open()) {
    return EmulatorResult::Failure("Could not open ROM file for header read: " + rom_path);
  }

  out_info.game_title = ReadFixedField(file, 0x0A0, 12);
  out_info.game_code = ReadFixedField(file, 0x0AC, 4);
  out_info.maker_code = ReadFixedField(file, 0x0B0, 2);

  return EmulatorResult::Success();
}

EmulatorResult RomLoader::Load(IEmulatorCore& core, const std::string& rom_path) {
  const EmulatorResult validation = ValidatePath(rom_path);
  if (!validation) {
    return validation;
  }
  return core.LoadRom(rom_path);
}

}  // namespace unboundmp::emulator
