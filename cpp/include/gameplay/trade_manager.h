#pragma once
#include <cstdint>
#include "gameplay/action_manager.h"

namespace unboundmp::gameplay {

class TradeManager {
public:
    TradeManager() = default;
    ~TradeManager() = default;

    void Initialize(ActionManager* action_mgr);
    
    bool StartTrade();
    void OnTradeSessionStarted(uint32_t session_id);
    void OnTradeSessionEnded(uint32_t session_id, uint8_t reason);
    
    enum class TradeState { kIdle, kRequesting, kActive };
    TradeState GetState() const { return state_; }

private:
    ActionManager* action_mgr_ = nullptr;
    TradeState state_ = TradeState::kIdle;
};

} // namespace unboundmp::gameplay
