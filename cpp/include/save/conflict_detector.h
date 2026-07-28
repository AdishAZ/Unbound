#pragma once

#include <string>

#include "save/save_types.h"

namespace unboundmp::save {

// Detects whether a save file changed in a way SaveManager did not itself
// cause. This is single-client, single-file conflict detection (there is
// no shared/cloud save in this project - see README "explicitly out of
// scope: shared inventory" - saves stay local per player) aimed at two
// real failure modes:
//   1. Two client processes (or two instances of this client) pointed at
//      the same .sav path - e.g. the user accidentally launched the game
//      twice. Whichever one writes second silently clobbers the other's
//      trade/battle result.
//   2. An external tool (a save editor, a sync/backup daemon, cloud
//      storage client) rewrites the file out from under a running client.
// Neither case is preventable from here (there's no file locking that
// survives a crash, and GBA saves are plain files by design), but both are
// detectable: SaveManager records a SaveSnapshot at points where it knows
// the file's expected state, and ConflictDetector compares that against
// the file's actual current state.
class ConflictDetector {
 public:
  // Takes a fresh snapshot of the file at `save_path`.
  static SaveSnapshot Snapshot(const std::string& save_path);

  // Compares `expected` (a previously taken snapshot, believed to still
  // describe the file) against the file's current on-disk state. Returns
  // ConflictReport::None() if they match. Does not distinguish *who*
  // changed the file - only that it changed when SaveManager did not
  // record having changed it itself.
  static ConflictReport Compare(const SaveSnapshot& expected, const std::string& save_path);
};

}  // namespace unboundmp::save
