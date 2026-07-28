#pragma once

#include <cstdint>
#include <deque>

#include "packets.pb.h"

namespace unboundmp::gameplay {

// One tile the trainer has occupied, plus the direction they were facing
// while moving into it.
struct TrailWaypoint {
  int32_t x = 0;
  int32_t y = 0;
  protocol::Direction direction = protocol::DIRECTION_UNSPECIFIED;

  bool operator==(const TrailWaypoint& other) const {
    return x == other.x && y == other.y;
  }
};

// Records the tiles a trainer has walked over and answers "where should
// the follower Pokemon be walking toward right now" - a fixed number of
// tiles (`lag_steps`) behind the trainer's current tile.
//
// This is the classic Gen IV-onward "following Pokemon" offset algorithm
// (HeartGold/SoulSilver, and Unbound's overworld follower feature is built
// on the same idea): the follower doesn't path independently, it just
// walks into the tile the trainer occupied `lag_steps` moves ago. With
// lag_steps == 1 the follower is always exactly one tile behind, which is
// the default here; a config knob rather than a hardcoded 1 in case
// reverse engineering or playtesting finds Unbound's follower should trail
// by more than one tile (e.g. to visually clear a diagonal corner) or a
// future multi-follower feature wants staggered lags.
class MovementTrail {
 public:
  explicit MovementTrail(size_t lag_steps = 1, size_t max_history = 8)
      : lag_steps_(lag_steps), max_history_(max_history) {}

  // Records that the trainer has entered tile (x, y), having moved in
  // `direction_moved` to get there. A no-op if this tile is identical to
  // the most recently recorded one - repeated PlayerStateUpdates for a
  // stationary trainer (e.g. periodic WorldSnapshot resync) must not
  // inflate the trail, or the follower would appear to take extra
  // "phantom" steps it never actually walked.
  void RecordTile(int32_t x, int32_t y, protocol::Direction direction_moved) {
    TrailWaypoint waypoint{x, y, direction_moved};
    if (!history_.empty() && history_.back() == waypoint) {
      return;
    }
    history_.push_back(waypoint);
    while (history_.size() > max_history_) {
      history_.pop_front();
    }
  }

  // Discards all recorded history. Used on map transitions (see
  // follower_manager.h) so the follower doesn't try to walk a straight
  // line across a map boundary it never actually crossed on foot - it
  // should just reappear beside the trainer on the new map instead.
  void Reset() { history_.clear(); }

  bool HasHistory() const { return !history_.empty(); }

  // The tile the follower should currently be walking toward. If fewer
  // than `lag_steps_ + 1` tiles have been recorded yet (e.g. right after
  // joining, or just after a map transition), returns the oldest tile
  // known rather than failing - this means a freshly-spawned follower
  // trails by less than the full lag until enough history accumulates,
  // rather than not knowing where to stand at all.
  TrailWaypoint FollowerTarget() const {
    if (history_.empty()) {
      return TrailWaypoint{};
    }
    if (history_.size() <= lag_steps_) {
      return history_.front();
    }
    return history_[history_.size() - 1 - lag_steps_];
  }

 private:
  std::deque<TrailWaypoint> history_;
  size_t lag_steps_;
  size_t max_history_;
};

}  // namespace unboundmp::gameplay
