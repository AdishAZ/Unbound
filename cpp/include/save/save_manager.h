#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "emulator/emulator_core.h"
#include "save/backup_manager.h"
#include "save/save_types.h"

namespace unboundmp::save {

// Owns the lifecycle of "is this player's save file correct on disk",
// bracketing trade/battle link sessions with backups and verifying they
// actually persisted. This is intentionally scoped to a single local save
// file - see README's "explicitly out of scope: shared inventory" - there
// is no cross-player save merging here. What trading/battling changes is
// each participant's own save, exactly as it would from a real link cable
// session; this layer's job is only to make sure that change reliably
// reaches disk and survives.
//
// Does not read ROM data, does not modify the ROM, and does not read
// Pokemon/save internal structure - it only observes the .sav file's raw
// bytes as an opaque blob (via ConflictDetector/hashing), matching every
// other layer's "outside the ROM" contract. It also does not perform any
// memory reads or writes into the running emulator core; hooking this up
// to IEmulatorCore's own save flush (if/when the mgba backend exposes one)
// and to NetworkManager's LinkSessionEnd handler is future integration
// work, not part of this layer.
//
// Thread-safety: public methods are safe to call from any single thread
// consistently; if a future milestone calls this from both a network
// callback thread and a UI thread, callers must serialize their own calls
// (a simple mutex is used internally only to protect this object's own
// bookkeeping map, not to serialize the two threads' logical operations).
class SaveManager {
 public:
  explicit SaveManager(int max_backups_per_save = 30);

  // Resolves the save path via SaveLoader's convention (sibling .sav next
  // to the ROM), validates it, and records a baseline snapshot. Also takes
  // an automatic "session_start" backup if a save file already exists
  // (nothing to back up yet is not an error - a brand new save is fine).
  emulator::EmulatorResult InitializeForRom(const std::string& rom_path);

  // Same, but with an explicit save path instead of deriving one from a
  // ROM path.
  emulator::EmulatorResult InitializeForSavePath(const std::string& save_path);

  const std::string& save_path() const { return save_path_; }

  // Call before starting a trade/battle link session. Takes a "pre_<kind>"
  // backup and remembers this session's starting snapshot so EndLinkSession
  // can later tell whether anything actually changed.
  emulator::EmulatorResult BeginLinkSession(uint32_t session_id, LinkKind kind);

  // Call when a trade/battle link session ends. `completed` should be true
  // only for LINK_SESSION_END_REASON_COMPLETED (see packets.proto) - for
  // any other end reason (cancelled/peer_left/timeout) pass false, which
  // still takes a diagnostic backup and clears session bookkeeping but does
  // not treat "save unchanged" as an error, since nothing was expected to
  // change.
  SaveSyncResult EndLinkSession(uint32_t session_id, LinkKind kind, bool completed);

  // Compares the current save file against the last snapshot SaveManager
  // itself recorded (from InitializeFor*/BeginLinkSession/EndLinkSession).
  // Call periodically (e.g. once per second on an idle/UI tick) to catch
  // external modification between sessions, not just around them.
  ConflictReport CheckForExternalConflict();

  // Takes an ad-hoc backup outside of a link session (e.g. a manual "save
  // backup now" UI action, or a periodic timer). `tag` is any short label;
  // "manual" and "periodic" are reasonable choices.
  SaveSyncResult BackupNow(const std::string& tag);

  std::vector<BackupEntry> ListBackups() const;
  emulator::EmulatorResult RestoreBackup(const BackupEntry& backup);

  BackupManager& backup_manager() { return backups_; }

 private:
  BackupManager backups_;
  std::string save_path_;
  SaveSnapshot last_known_snapshot_;
  bool initialized_ = false;

  mutable std::mutex mutex_;
  struct PendingSession {
    LinkKind kind;
    SaveSnapshot pre_session_snapshot;
  };
  std::unordered_map<uint32_t, PendingSession> pending_sessions_;

  SaveSyncResult MakeCheckpointBackup(const std::string& tag);
};

}  // namespace unboundmp::save
