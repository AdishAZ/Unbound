#pragma once

#include <cstdint>
#include "parser/domain_types.h"
#include "models/inventory.h"

namespace unboundmp::parser {
class ParserRegistry;
}

namespace unboundmp::game_state {

enum DirtyFlags : uint32_t {
  kNone = 0,
  kPosition = 1 << 0,
  kMap = 1 << 1,
  kDirection = 1 << 2,
  kMovement = 1 << 3,
  kParty = 1 << 4,
  kFollower = 1 << 5,
  kInventory = 1 << 6,
  kEventObjects = 1 << 7,
  kAll = 0xFFFFFFFF
};

inline DirtyFlags operator|(DirtyFlags a, DirtyFlags b) {
  return static_cast<DirtyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline DirtyFlags operator&(DirtyFlags a, DirtyFlags b) {
  return static_cast<DirtyFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline DirtyFlags& operator|=(DirtyFlags& a, DirtyFlags b) {
  a = a | b;
  return a;
}

inline DirtyFlags& operator&=(DirtyFlags& a, DirtyFlags b) {
  a = a & b;
  return a;
}

class GameState {
 public:
  void Update(parser::ParserRegistry& parsers, int64_t frame_count);
  
  const parser::LocalPlayerSnapshot& GetLocalPlayer() const { return local_player_; }
  const parser::RawPartyData& GetParty() const { return party_; }
  const parser::FollowerInfo& GetFollower() const { return follower_; }
  const models::Inventory& GetInventory() const { return inventory_; }
  const parser::RawEventObjectData& GetEventObjects() const { return event_objects_; }
  
  DirtyFlags GetDirtyFlags() const { return dirty_flags_; }
  
  DirtyFlags ConsumeDirtyFlags() {
    DirtyFlags flags = dirty_flags_;
    dirty_flags_ = kNone;
    return flags;
  }
  
  bool IsValid() const { return is_valid_; }
  int64_t GetFrameCount() const { return frame_count_; }

 private:
  parser::LocalPlayerSnapshot local_player_{};
  parser::RawPartyData party_{};
  parser::FollowerInfo follower_{};
  models::Inventory inventory_{};
  parser::RawEventObjectData event_objects_{};
  DirtyFlags dirty_flags_ = kNone;
  int64_t frame_count_ = 0;
  bool is_valid_ = false;
};

} // namespace unboundmp::game_state
