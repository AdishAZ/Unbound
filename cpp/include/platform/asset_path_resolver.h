#pragma once

#include <string>
#include <vector>

namespace unboundmp::platform {

// Where a given kind of file conventionally lives. Desktop keeps
// everything next to the ROM the user pointed the client at (see
// SaveLoader::DefaultSavePathFor); Android has no such concept - there's
// no "next to the ROM" location an app can always write to; a ROM opened
// via the Storage Access Framework may not even resolve to a real
// filesystem path at all - so the app needs its own writable directories
// for save/backup/config data, plus a place to cache a copy of a
// SAF-provided ROM if the emulator core needs a real path rather than a
// file descriptor.
enum class AssetKind {
  kRom,
  kSave,
  kBackups,
  kConfig,
};

// A resolved root directory for one AssetKind, plus whether the caller
// still needs to derive a filename within it (ROM/save are usually a
// specific file the user picked or that matches the ROM's name; backups/
// config are directories the resolver fully owns).
struct AssetLocation {
  std::string root_dir;
  bool caller_supplies_filename = true;
};

// Pure path-string logic - never touches the filesystem itself (no
// exists()/create_directories() calls here; BackupManager/SaveLoader own
// actually creating directories, exactly like the render layer computes
// geometry without ever opening a window). This is what makes it testable
// on desktop CI without an Android device/emulator: given the same inputs,
// AssetPathResolver returns the same paths whether it's actually running
// on Android or being exercised by asset_path_resolver_example on Linux.
class AssetPathResolver {
 public:
  // `app_private_dir` is the platform's app-private writable root:
  //   - Desktop: typically unused (kRom/kSave resolve relative to a
  //     user-chosen ROM path instead, via ForRomSibling() below).
  //   - Android: Context.getFilesDir() or getExternalFilesDir(null),
  //     passed down through the JNI bridge (android/app/src/main/cpp) -
  //     this class never calls into JNI/Android APIs itself.
  explicit AssetPathResolver(std::string app_private_dir);

  // Desktop-style resolution: save/backups live next to the ROM file,
  // exactly as SaveLoader/BackupManager already assume. Kept here too so
  // callers can go through one resolver on any platform rather than
  // branching themselves.
  static AssetLocation ForRomSibling(const std::string& rom_path, AssetKind kind);

  // App-sandboxed resolution: save/backups/config live under subdirectories
  // of app_private_dir, namespaced by a caller-supplied `rom_identifier`
  // (e.g. a hash or filename of the ROM the user picked via SAF) so
  // multiple ROMs' saves never collide in the app's private storage.
  AssetLocation ForAppPrivateStorage(AssetKind kind, const std::string& rom_identifier) const;

  // If a ROM was opened via a content:// URI (Android SAF) rather than a
  // real filesystem path, the emulator core still needs a real path to
  // open. This returns the conventional cache location to copy the ROM's
  // bytes into once (read-only, never re-derived from or written back to
  // the SAF URI - the ROM stays byte-for-byte identical, same "never
  // modify the ROM" contract as RomLoader on desktop).
  std::string RomCachePathFor(const std::string& rom_identifier) const;

 private:
  std::string app_private_dir_;

  std::string SubdirFor(AssetKind kind, const std::string& rom_identifier) const;
};

}  // namespace unboundmp::platform
