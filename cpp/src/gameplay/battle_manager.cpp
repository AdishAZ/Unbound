#include "gameplay/battle_manager.h"

namespace unboundmp::gameplay {

void BattleManager::Initialize(ActionManager* action_mgr) {
    action_mgr_ = action_mgr;
}

bool BattleManager::StartBattle() {
    if (!action_mgr_) return false;
    state_ = BattleState::kRequesting;
    return action_mgr_->RequestAction(ActionType::kBattle);
}

void BattleManager::OnBattleSessionStarted(uint32_t session_id) {
    state_ = BattleState::kActive;
}

void BattleManager::OnBattleSessionEnded(uint32_t session_id, uint8_t reason) {
    state_ = BattleState::kIdle;
}

} // namespace unboundmp::gameplay
