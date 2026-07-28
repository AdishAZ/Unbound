#pragma once
#include <cstdint>
#include "gameplay/action_manager.h"

namespace unboundmp::gameplay {

class BattleManager {
public:
    BattleManager() = default;
    ~BattleManager() = default;

    void Initialize(ActionManager* action_mgr);
    
    bool StartBattle();
    void OnBattleSessionStarted(uint32_t session_id);
    void OnBattleSessionEnded(uint32_t session_id, uint8_t reason);
    
    enum class BattleState { kIdle, kRequesting, kActive };
    BattleState GetState() const { return state_; }

private:
    ActionManager* action_mgr_ = nullptr;
    BattleState state_ = BattleState::kIdle;
};

} // namespace unboundmp::gameplay
