#include "save/conflict_detector.h"

#include <chrono>
#include <filesystem>

#include "save/hashing.h"

namespace unboundmp::save {

namespace fs = std::filesystem;

SaveSnapshot ConflictDetector::Snapshot(const std::string& save_path) {
  SaveSnapshot snap;
  uint64_t size = 0;
  snap.content_hash = HashFile(save_path, &snap.existed, &size);
  snap.size_bytes = size;

  std::error_code ec;
  if (snap.existed) {
    auto ftime = fs::last_write_time(save_path, ec);
    if (!ec) {
      using namespace std::chrono;

      // MSVC-compatible conversion from file_clock to system_clock
      auto sctp = system_clock::now() +
          (ftime - fs::file_time_type::clock::now());

      snap.mtime_unix_seconds =
          duration_cast<seconds>(sctp.time_since_epoch()).count();
    }
  }

  return snap;
}

ConflictReport ConflictDetector::Compare(const SaveSnapshot& expected,
                                          const std::string& save_path) {
  const SaveSnapshot current = Snapshot(save_path);

  if (expected.existed && !current.existed) {
    return ConflictReport::Detected("Save file disappeared: " + save_path);
  }
  if (!expected.existed && current.existed) {
    // Not necessarily a conflict on its own (first-ever save is expected
    // to go from "doesn't exist" to "exists"), but SaveManager only calls
    // Compare() at points where it expects the file to be unchanged, so
    // treat it as one here.
    return ConflictReport::Detected("Save file appeared unexpectedly: " + save_path);
  }
  if (expected.existed && current.existed && expected.content_hash != current.content_hash) {
    return ConflictReport::Detected(
        "Save file contents changed unexpectedly (external write to " + save_path + ")");
  }
  return ConflictReport::None();
}

}  // namespace unboundmp::save
