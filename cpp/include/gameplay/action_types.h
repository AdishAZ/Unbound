#pragma once

#include <cstdint>

namespace unboundmp::gameplay {

enum class ActionType {
  kTrade,
  kBattle
};

enum class ActionState {
  kIdle,
  kPending,
  kActive
};

struct ActionRequest {
  ActionType type;
  uint64_t target_account_id;
};

} // namespace unboundmp::gameplay
