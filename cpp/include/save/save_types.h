#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace unboundmp::save {

// What kind of link session a save checkpoint is bracketing. Deliberately a
// standalone enum rather than reusing protocol::LinkMode - this layer never
// touches the network protocol directly (same decoupling pattern as
// interaction/, which is built on plain structs instead of protobuf types).
// A future milestone maps protocol::LinkMode -> save::LinkKind at the call
// site where NetworkManager's LinkSessionEnd handler invokes SaveManager.
enum class LinkKind {
  kUnspecified = 0,
  kTrade = 1,
  kBattle = 2,
};

const char* ToString(LinkKind kind);

// A point-in-time fingerprint of a save file, cheap enough to take often.
// Two snapshots with different `content_hash` values reflect different save
// bytes; `size_bytes` and `mtime` are kept alongside for diagnostics and for
// the "nothing on disk yet" case (size_bytes == 0 and content_hash == 0 with
// existed == false).
struct SaveSnapshot {
  bool existed = false;
  uint64_t content_hash = 0;
  uint64_t size_bytes = 0;
  int64_t mtime_unix_seconds = 0;

  bool operator==(const SaveSnapshot& other) const {
    return existed == other.existed && content_hash == other.content_hash &&
           size_bytes == other.size_bytes;
  }
  bool operator!=(const SaveSnapshot& other) const { return !(*this == other); }
};

// Result of a conflict check: did the save file on disk change in a way the
// SaveManager did not itself cause (e.g. another process/instance writing
// the same .sav path, or the file being removed/replaced externally)?
struct ConflictReport {
  bool conflict = false;
  std::string detail;

  static ConflictReport None() { return {false, ""}; }
  static ConflictReport Detected(std::string why) { return {true, std::move(why)}; }
};

// A single rotated backup file on disk.
struct BackupEntry {
  std::string path;
  std::string tag;               // e.g. "pre_trade", "post_battle_completed"
  int64_t created_unix_seconds = 0;
  uint64_t size_bytes = 0;
};

// Outcome of bracketing a link session with save checkpoints. `persisted`
// answers the actual milestone goal - "trades and battles permanently
// affect saves" - by comparing the pre- and post-session file hashes.
struct SaveSyncResult {
  bool ok = false;
  std::string message;
  bool persisted = false;  // true if the save file's contents actually changed
  std::optional<BackupEntry> backup;
  ConflictReport conflict = ConflictReport::None();

  static SaveSyncResult Failure(std::string msg) {
    SaveSyncResult r;
    r.ok = false;
    r.message = std::move(msg);
    return r;
  }
};

}  // namespace unboundmp::save
