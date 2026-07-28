#include "platform/asset_path_resolver.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace unboundmp::platform {

namespace fs = std::filesystem;

namespace {

// Filesystem-safe version of a caller-supplied identifier (e.g. a ROM
// filename), used as a subdirectory name. Pure string transform, same
// spirit as BackupManager's tag sanitization.
std::string SanitizeIdentifier(const std::string& identifier) {
  // '.' is deliberately excluded (not just '/'): this identifier becomes a
  // path *segment* used as a directory name, and allowing dots through
  // would let a crafted identifier like "../../etc" collapse into a path
  // traversal sequence after sanitization ("_.._.._etc" is safe, but
  // preserving the dots would not be). The identifier is a namespace key,
  // not a filename, so it never needs to keep an extension.
  std::string clean;
  clean.reserve(identifier.size());
  for (char c : identifier) {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
      clean.push_back(c);
    } else {
      clean.push_back('_');
    }
  }
  return clean.empty() ? "default" : clean;
}

}  // namespace

AssetPathResolver::AssetPathResolver(std::string app_private_dir)
    : app_private_dir_(std::move(app_private_dir)) {}

AssetLocation AssetPathResolver::ForRomSibling(const std::string& rom_path, AssetKind kind) {
  fs::path rom(rom_path);
  fs::path parent = rom.has_parent_path() ? rom.parent_path() : fs::path(".");

  switch (kind) {
    case AssetKind::kRom:
      return {parent.string(), /*caller_supplies_filename=*/true};
    case AssetKind::kSave:
      return {parent.string(), /*caller_supplies_filename=*/true};
    case AssetKind::kBackups:
      return {(parent / "backups").string(), /*caller_supplies_filename=*/false};
    case AssetKind::kConfig:
      return {parent.string(), /*caller_supplies_filename=*/true};
  }
  return {parent.string(), true};
}

std::string AssetPathResolver::SubdirFor(AssetKind kind, const std::string& rom_identifier) const {
  const std::string clean_id = SanitizeIdentifier(rom_identifier);
  fs::path root(app_private_dir_);

  switch (kind) {
    case AssetKind::kRom:
      return (root / "rom_cache").string();
    case AssetKind::kSave:
      return (root / "saves" / clean_id).string();
    case AssetKind::kBackups:
      return (root / "saves" / clean_id / "backups").string();
    case AssetKind::kConfig:
      return (root / "config").string();
  }
  return root.string();
}

AssetLocation AssetPathResolver::ForAppPrivateStorage(AssetKind kind,
                                                       const std::string& rom_identifier) const {
  const bool caller_supplies_filename = (kind != AssetKind::kBackups);
  return {SubdirFor(kind, rom_identifier), caller_supplies_filename};
}

std::string AssetPathResolver::RomCachePathFor(const std::string& rom_identifier) const {
  fs::path cache_dir(SubdirFor(AssetKind::kRom, rom_identifier));
  fs::path filename = fs::path(SanitizeIdentifier(rom_identifier));
  if (filename.extension() != ".gba") {
    filename += ".gba";
  }
  return (cache_dir / filename).string();
}

}  // namespace unboundmp::platform
