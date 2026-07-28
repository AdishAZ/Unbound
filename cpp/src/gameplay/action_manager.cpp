#include "gameplay/action_manager.h"
#include "interaction/interaction_manager.h"
#include "interaction/player_selector.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"

namespace unboundmp::gameplay {

void ActionManager::Initialize(interaction::InteractionManager* interaction, interaction::PlayerSelector* selector, network::MultiplayerClient* client) {
    interaction_ = interaction;
    selector_ = selector;
    client_ = client;
}

bool ActionManager::RequestAction(ActionType type) {
    if (state_ != ActionState::kIdle) {
        return false;
    }
    
    if (!selector_ || !interaction_ || !client_) {
        return false;
    }

    uint32_t local_player_id = 0;
    auto target_opt = selector_->GetSelection(local_player_id, *interaction_);
    if (!target_opt) {
        target_opt = selector_->ResolveTarget(local_player_id, *interaction_);
    }
    
    if (!target_opt) {
        return false;
    }
    
    pending_target_ = *target_opt;
    pending_type_ = type;
    
    network::LinkSessionRequestPacket request;
    request.target_account_id = pending_target_;
    request.mode = (type == ActionType::kTrade) ? network::LinkSessionMode::kTrade : network::LinkSessionMode::kBattle;
    
    network::Packet pkt;
    pkt.type = network::PacketType::kLinkSessionRequest;
    pkt.payload = request.Serialize();
    client_->SendPacket(pkt);
    
    state_ = ActionState::kPending;
    return true;
}

void ActionManager::CancelPendingAction() {
    if (state_ == ActionState::kPending) {
        state_ = ActionState::kIdle;
        pending_target_ = 0;
    }
}

void ActionManager::OnLinkSessionResponse(bool accepted, uint32_t session_id, std::string /*reject_reason*/) {
    if (state_ != ActionState::kPending) {
        return;
    }
    
    if (accepted) {
        state_ = ActionState::kActive;
        active_session_id_ = session_id;
    } else {
        state_ = ActionState::kIdle;
        pending_target_ = 0;
    }
}

void ActionManager::Update() {
    // Polling logic would go here if needed
}

} // namespace unboundmp::gameplay
