#pragma once

#include <cstdint>
#include <string>
#include "gameplay/action_types.h"

namespace unboundmp::interaction {
class InteractionManager;
class PlayerSelector;
}
namespace unboundmp::network {
class MultiplayerClient;
}

namespace unboundmp::gameplay {

class ActionManager {
 public:
  ActionManager() = default;
  void Initialize(interaction::InteractionManager* interaction, interaction::PlayerSelector* selector, network::MultiplayerClient* client);
  
  bool RequestAction(ActionType type);
  void CancelPendingAction();
  
  ActionState GetState() const { return state_; }
  uint64_t GetPendingTarget() const { return pending_target_; }
  
  void OnLinkSessionResponse(bool accepted, uint32_t session_id, std::string reject_reason);
  void Update();

 private:
  interaction::InteractionManager* interaction_ = nullptr;
  interaction::PlayerSelector* selector_ = nullptr;
  network::MultiplayerClient* client_ = nullptr;
  
  ActionState state_ = ActionState::kIdle;
  uint64_t pending_target_ = 0;
  ActionType pending_type_ = ActionType::kTrade;
  uint32_t active_session_id_ = 0;
};

} // namespace unboundmp::gameplay
