#include "save/save_manager.h"

#include "emulator/save_loader.h"
#include "save/conflict_detector.h"

namespace unboundmp::save {

using emulator::EmulatorResult;
using emulator::SaveLoader;

const char* ToString(LinkKind kind) {
  switch (kind) {
    case LinkKind::kTrade: return "trade";
    case LinkKind::kBattle: return "battle";
    case LinkKind::kUnspecified: default: return "unspecified";
  }
}

SaveManager::SaveManager(int max_backups_per_save) : backups_(max_backups_per_save) {}

EmulatorResult SaveManager::InitializeForRom(const std::string& rom_path) {
  return InitializeForSavePath(SaveLoader::DefaultSavePathFor(rom_path));
}

EmulatorResult SaveManager::InitializeForSavePath(const std::string& save_path) {
  const EmulatorResult validation = SaveLoader::ValidatePath(save_path);
  if (!validation) {
    return validation;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  save_path_ = save_path;
  last_known_snapshot_ = ConflictDetector::Snapshot(save_path_);
  initialized_ = true;

  // A fresh save (nothing on disk yet) has nothing to back up - that's
  // expected on first launch, not an error.
  if (last_known_snapshot_.existed) {
    BackupEntry entry;
    backups_.CreateBackup(save_path_, "session_start", &entry);
    // Best-effort: a failed startup backup shouldn't block initialization
    // (e.g. a read-only backups dir on first run) - InitializeFor* still
    // succeeds since the save itself was validated above. Later calls to
    // BackupNow()/EndLinkSession() will surface backup failures directly.
  }

  return EmulatorResult::Success();
}

EmulatorResult SaveManager::BeginLinkSession(uint32_t session_id, LinkKind kind) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!initialized_) {
    return EmulatorResult::Failure("SaveManager not initialized");
  }

  const SaveSnapshot pre_snapshot = ConflictDetector::Snapshot(save_path_);
  last_known_snapshot_ = pre_snapshot;
  pending_sessions_[session_id] = PendingSession{kind, pre_snapshot};

  if (pre_snapshot.existed) {
    BackupEntry entry;
    const std::string tag = std::string("pre_") + ToString(kind);
    EmulatorResult backup_result = backups_.CreateBackup(save_path_, tag, &entry);
    if (!backup_result) {
      return EmulatorResult::Failure("Failed to create pre-session backup: " +
                                      backup_result.message);
    }
  }
  return EmulatorResult::Success();
}

SaveSyncResult SaveManager::EndLinkSession(uint32_t session_id, LinkKind kind, bool completed) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!initialized_) {
    return SaveSyncResult::Failure("SaveManager not initialized");
  }

  SaveSnapshot pre_snapshot;
  auto it = pending_sessions_.find(session_id);
  if (it != pending_sessions_.end()) {
    pre_snapshot = it->second.pre_session_snapshot;
    pending_sessions_.erase(it);
  } else {
    // EndLinkSession without a matching BeginLinkSession (e.g. process
    // restarted mid-session) - fall back to the last known snapshot so we
    // can still take a diagnostic backup and report *something* useful
    // rather than failing outright.
    pre_snapshot = last_known_snapshot_;
  }

  const SaveSnapshot post_snapshot = ConflictDetector::Snapshot(save_path_);

  SaveSyncResult result;
  result.ok = true;

  const std::string tag =
      std::string("post_") + ToString(kind) + (completed ? "_completed" : "_ended");

  if (post_snapshot.existed) {
    BackupEntry entry;
    EmulatorResult backup_result = backups_.CreateBackup(save_path_, tag, &entry);
    if (backup_result) {
      result.backup = entry;
    } else {
      result.ok = false;
      result.message = "Failed to create post-session backup: " + backup_result.message;
    }
  }

  if (completed) {
    result.persisted = (post_snapshot != pre_snapshot) && post_snapshot.existed;
    if (!result.persisted) {
      result.ok = false;
      result.message = result.message.empty()
          ? std::string(ToString(kind)) +
                " completed but the save file did not change - result may not have "
                "persisted to disk"
          : result.message;
    }
  } else {
    // Session did not complete (cancelled/peer left/timeout) - the save is
    // expected to be unchanged; that's success, not a persistence failure.
    result.persisted = false;
  }

  last_known_snapshot_ = post_snapshot;
  return result;
}

ConflictReport SaveManager::CheckForExternalConflict() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!initialized_) {
    return ConflictReport::Detected("SaveManager not initialized");
  }
  ConflictReport report = ConflictDetector::Compare(last_known_snapshot_, save_path_);
  // Whether or not there was a conflict, the current state becomes the new
  // baseline - CheckForExternalConflict() reports *changes since last
  // check*, not a running diff against session start.
  last_known_snapshot_ = ConflictDetector::Snapshot(save_path_);
  return report;
}

SaveSyncResult SaveManager::BackupNow(const std::string& tag) {
  std::lock_guard<std::mutex> lock(mutex_);
  return MakeCheckpointBackup(tag);
}

SaveSyncResult SaveManager::MakeCheckpointBackup(const std::string& tag) {
  if (!initialized_) {
    return SaveSyncResult::Failure("SaveManager not initialized");
  }
  BackupEntry entry;
  EmulatorResult backup_result = backups_.CreateBackup(save_path_, tag, &entry);
  if (!backup_result) {
    return SaveSyncResult::Failure(backup_result.message);
  }
  SaveSyncResult result;
  result.ok = true;
  result.backup = entry;
  last_known_snapshot_ = ConflictDetector::Snapshot(save_path_);
  return result;
}

std::vector<BackupEntry> SaveManager::ListBackups() const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!initialized_) return {};
  return backups_.ListBackups(save_path_);
}

EmulatorResult SaveManager::RestoreBackup(const BackupEntry& backup) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!initialized_) {
    return EmulatorResult::Failure("SaveManager not initialized");
  }
  EmulatorResult result = backups_.RestoreBackup(backup, save_path_);
  if (result) {
    last_known_snapshot_ = ConflictDetector::Snapshot(save_path_);
  }
  return result;
}

}  // namespace unboundmp::save
