// Standalone smoke test for the Android-support asset path resolver.
// Pure string logic - no filesystem calls, no Android Context, no JNI -
// so it runs identically here as it will on-device once the JNI bridge
// passes down a real app-private directory.
#include <iostream>

#include "platform/asset_path_resolver.h"

int main() {
  using namespace unboundmp::platform;

  // --- Desktop-style: everything next to the ROM -------------------------
  const std::string rom_path = "/home/player/roms/pokemon_unbound.gba";

  auto rom_loc = AssetPathResolver::ForRomSibling(rom_path, AssetKind::kRom);
  auto save_loc = AssetPathResolver::ForRomSibling(rom_path, AssetKind::kSave);
  auto backups_loc = AssetPathResolver::ForRomSibling(rom_path, AssetKind::kBackups);

  std::cout << "Desktop ROM dir:     " << rom_loc.root_dir << "\n";
  std::cout << "Desktop save dir:    " << save_loc.root_dir << "\n";
  std::cout << "Desktop backups dir: " << backups_loc.root_dir << " (caller_supplies_filename="
            << backups_loc.caller_supplies_filename << ")\n";

  if (save_loc.root_dir != "/home/player/roms") {
    std::cout << "BUG: expected save dir to sit next to the ROM\n";
    return 1;
  }
  if (backups_loc.root_dir != "/home/player/roms/backups") {
    std::cout << "BUG: expected backups dir to be a 'backups' sibling\n";
    return 1;
  }

  // --- Android-style: app-private sandboxed storage -----------------------
  AssetPathResolver android_resolver("/data/user/0/com.unboundmp.client/files");
  const std::string rom_identifier = "pokemon_unbound.gba";  // e.g. SAF display name

  auto android_save = android_resolver.ForAppPrivateStorage(AssetKind::kSave, rom_identifier);
  auto android_backups =
      android_resolver.ForAppPrivateStorage(AssetKind::kBackups, rom_identifier);
  auto android_config = android_resolver.ForAppPrivateStorage(AssetKind::kConfig, rom_identifier);
  const std::string rom_cache_path = android_resolver.RomCachePathFor(rom_identifier);

  std::cout << "\nAndroid save dir:    " << android_save.root_dir << "\n";
  std::cout << "Android backups dir: " << android_backups.root_dir << "\n";
  std::cout << "Android config dir:  " << android_config.root_dir << "\n";
  std::cout << "Android ROM cache:   " << rom_cache_path << "\n";

  if (android_save.root_dir.find("com.unboundmp.client/files/saves/pokemon_unbound_gba") ==
      std::string::npos) {
    std::cout << "BUG: expected save dir to be namespaced by ROM identifier\n";
    return 1;
  }

  // Two different ROMs must never collide.
  auto other_save = android_resolver.ForAppPrivateStorage(AssetKind::kSave, "other_rom.gba");
  std::cout << "Second ROM's save dir: " << other_save.root_dir << "\n";
  if (other_save.root_dir == android_save.root_dir) {
    std::cout << "BUG: two different ROMs resolved to the same save directory\n";
    return 1;
  }

  // Identifiers with unsafe characters (a SAF display name can contain
  // almost anything) get sanitized into a safe subdirectory name rather
  // than producing a broken/unsafe path.
  auto weird_id_save =
      android_resolver.ForAppPrivateStorage(AssetKind::kSave, "My Save/../../etc/passwd");
  std::cout << "Sanitized weird identifier -> " << weird_id_save.root_dir << "\n";
  if (weird_id_save.root_dir.find("..") != std::string::npos) {
    std::cout << "BUG: sanitized identifier must not contain path traversal sequences\n";
    return 1;
  }

  std::cout << "\nAll asset_path checks passed.\n";
  return 0;
}
