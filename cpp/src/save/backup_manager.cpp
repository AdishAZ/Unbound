#include "save/backup_manager.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace unboundmp::save {

namespace fs = std::filesystem;
using emulator::EmulatorResult;

namespace {

int64_t NowUnixSeconds() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

// True copy of file bytes, not a hardlink/rename - backups must survive the
// live save file being truncated/rewritten in place, which a hardlink would
// not protect against.
bool CopyFileBytes(const std::string& from, const std::string& to) {
  std::error_code ec;
  fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
  return !ec;
}

}  // namespace

BackupManager::BackupManager(int max_backups_per_save)
    : max_backups_per_save_(max_backups_per_save) {}

std::string BackupManager::SanitizeTag(const std::string& tag) {
  std::string clean;
  clean.reserve(tag.size());
  for (char c : tag) {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
      clean.push_back(c);
    } else {
      clean.push_back('_');
    }
  }
  return clean.empty() ? "backup" : clean;
}

std::string BackupManager::SaveStem(const std::string& save_path) {
  return fs::path(save_path).stem().string();
}

std::string BackupManager::BackupDirFor(const std::string& save_path) {
  fs::path save(save_path);
  fs::path parent = save.has_parent_path() ? save.parent_path() : fs::path(".");
  fs::path backups = parent / "backups";

  std::error_code ec;
  fs::create_directories(backups, ec);
  return backups.string();
}

EmulatorResult BackupManager::CreateBackup(const std::string& save_path, const std::string& tag,
                                            BackupEntry* out_entry) {
  std::error_code ec;
  if (!fs::exists(save_path, ec) || !fs::is_regular_file(save_path, ec)) {
    return EmulatorResult::Failure("Cannot back up a save that does not exist yet: " + save_path);
  }

  const std::string backup_dir = BackupDirFor(save_path);
  const std::string stem = SaveStem(save_path);
  const std::string clean_tag = SanitizeTag(tag);
  const int64_t now = NowUnixSeconds();

  std::ostringstream name;
  name << stem << "." << now << "." << clean_tag << ".sav";
  fs::path backup_path = fs::path(backup_dir) / name.str();

  // Extremely unlikely, but if two backups land in the same second with
  // the same tag, disambiguate rather than silently overwrite one.
  int suffix = 1;
  while (fs::exists(backup_path, ec)) {
    std::ostringstream retry;
    retry << stem << "." << now << "." << clean_tag << "." << suffix << ".sav";
    backup_path = fs::path(backup_dir) / retry.str();
    ++suffix;
  }

  if (!CopyFileBytes(save_path, backup_path.string())) {
    return EmulatorResult::Failure("Failed to write backup file: " + backup_path.string());
  }

  if (out_entry) {
    out_entry->path = backup_path.string();
    out_entry->tag = clean_tag;
    out_entry->created_unix_seconds = now;
    uint64_t size = 0;
    std::error_code size_ec;
    size = static_cast<uint64_t>(fs::file_size(backup_path, size_ec));
    out_entry->size_bytes = size_ec ? 0 : size;
  }

  PruneOldBackups(save_path);
  return EmulatorResult::Success();
}

std::vector<BackupEntry> BackupManager::ListBackups(const std::string& save_path) const {
  std::vector<BackupEntry> entries;
  const std::string backup_dir = BackupDirFor(save_path);
  const std::string stem = SaveStem(save_path);
  const std::string prefix = stem + ".";

  std::error_code ec;
  if (!fs::exists(backup_dir, ec)) return entries;

  for (const auto& dirent : fs::directory_iterator(backup_dir, ec)) {
    if (!dirent.is_regular_file()) continue;
    const std::string filename = dirent.path().filename().string();
    if (filename.rfind(prefix, 0) != 0) continue;  // not this save's backup
    if (dirent.path().extension() != ".sav") continue;

    // Parse "<stem>.<unix>.<tag>.sav" (tag may itself contain '.' after
    // sanitization it won't, but be tolerant regardless).
    std::string rest = filename.substr(prefix.size());
    // Strip trailing ".sav"
    if (rest.size() >= 4 && rest.substr(rest.size() - 4) == ".sav") {
      rest = rest.substr(0, rest.size() - 4);
    }
    auto dot = rest.find('.');
    if (dot == std::string::npos) continue;

    BackupEntry entry;
    entry.path = dirent.path().string();
    try {
      entry.created_unix_seconds = std::stoll(rest.substr(0, dot));
    } catch (...) {
      entry.created_unix_seconds = 0;
    }
    entry.tag = rest.substr(dot + 1);
    std::error_code size_ec;
    auto size = fs::file_size(dirent.path(), size_ec);
    entry.size_bytes = size_ec ? 0 : static_cast<uint64_t>(size);
    entries.push_back(std::move(entry));
  }

  std::sort(entries.begin(), entries.end(), [](const BackupEntry& a, const BackupEntry& b) {
    return a.created_unix_seconds > b.created_unix_seconds;  // newest first
  });
  return entries;
}

EmulatorResult BackupManager::RestoreBackup(const BackupEntry& backup,
                                             const std::string& save_path) {
  std::error_code ec;
  if (!fs::exists(backup.path, ec) || !fs::is_regular_file(backup.path, ec)) {
    return EmulatorResult::Failure("Backup file does not exist: " + backup.path);
  }

  // Safety net: preserve whatever is currently live before overwriting it,
  // if there is anything there to preserve.
  if (fs::exists(save_path, ec) && fs::is_regular_file(save_path, ec)) {
    BackupEntry safety;
    EmulatorResult safety_result = CreateBackup(save_path, "pre_restore", &safety);
    if (!safety_result) {
      return EmulatorResult::Failure("Refusing to restore: could not snapshot current save "
                                      "first (" +
                                      safety_result.message + ")");
    }
  }

  if (!CopyFileBytes(backup.path, save_path)) {
    return EmulatorResult::Failure("Failed to copy backup onto save path: " + save_path);
  }
  return EmulatorResult::Success();
}

void BackupManager::PruneOldBackups(const std::string& save_path) {
  if (max_backups_per_save_ <= 0) return;  // unlimited

  std::vector<BackupEntry> entries = ListBackups(save_path);  // newest first
  if (static_cast<int>(entries.size()) <= max_backups_per_save_) return;

  std::error_code ec;
  for (size_t i = static_cast<size_t>(max_backups_per_save_); i < entries.size(); ++i) {
    fs::remove(entries[i].path, ec);
  }
}

}  // namespace unboundmp::save
