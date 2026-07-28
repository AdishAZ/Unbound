#include "gameplay/trade_manager.h"

namespace unboundmp::gameplay {

void TradeManager::Initialize(ActionManager* action_mgr) {
    action_mgr_ = action_mgr;
}

bool TradeManager::StartTrade() {
    if (!action_mgr_) return false;
    state_ = TradeState::kRequesting;
    return action_mgr_->RequestAction(ActionType::kTrade);
}

void TradeManager::OnTradeSessionStarted(uint32_t session_id) {
    state_ = TradeState::kActive;
}

void TradeManager::OnTradeSessionEnded(uint32_t session_id, uint8_t reason) {
    state_ = TradeState::kIdle;
}

} // namespace unboundmp::gameplay
