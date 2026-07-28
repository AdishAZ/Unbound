#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "emulator/emulator_core.h"
#include "save/save_types.h"

namespace unboundmp::save {

// Rotating, tagged backups of a save file. Backups live in a `backups/`
// subdirectory next to the .sav file (never inside it, never touching the
// ROM directory's other contents) and are never the file that gets loaded
// into the emulator directly - RestoreBackup() always copies a backup back
// onto the live save path rather than repointing anything at the backup
// itself, so "which file is the save" never becomes ambiguous.
//
// File naming: <save-stem>.<unix-seconds>.<tag>.sav
// e.g. pokemon_unbound.1737590400.post_trade_completed.sav
class BackupManager {
 public:
  // `max_backups_per_save` bounds how many backups PruneOldBackups() keeps
  // for a given save file's stem (oldest deleted first). 0 means unlimited.
  explicit BackupManager(int max_backups_per_save = 30);

  // Returns the backups directory for a given save path, creating it if
  // necessary. Does not create or touch the save file itself.
  static std::string BackupDirFor(const std::string& save_path);

  // Copies `save_path`'s current bytes into a new tagged backup file.
  // Fails (EmulatorResult::Failure) if save_path does not exist yet - you
  // cannot back up a save that hasn't been created. `tag` should be a
  // short identifier with no path separators (e.g. "pre_trade",
  // "post_battle_completed", "manual", "session_start"); it is
  // sanitized defensively regardless.
  emulator::EmulatorResult CreateBackup(const std::string& save_path, const std::string& tag,
                                         BackupEntry* out_entry);

  // Lists backups for `save_path`, newest first.
  std::vector<BackupEntry> ListBackups(const std::string& save_path) const;

  // Copies `backup.path`'s bytes onto `save_path`, after first taking a
  // safety backup of whatever is currently at `save_path` (tagged
  // "pre_restore") so a bad restore is itself recoverable.
  emulator::EmulatorResult RestoreBackup(const BackupEntry& backup, const std::string& save_path);

  // Deletes the oldest backups for `save_path` beyond `max_backups_per_save`.
  // Called automatically at the end of CreateBackup(); exposed separately
  // so SaveManager can also run it on a timer/idle tick.
  void PruneOldBackups(const std::string& save_path);

  int max_backups_per_save() const { return max_backups_per_save_; }

 private:
  int max_backups_per_save_;

  static std::string SanitizeTag(const std::string& tag);
  static std::string SaveStem(const std::string& save_path);
};

}  // namespace unboundmp::save
