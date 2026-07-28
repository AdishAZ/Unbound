#include "gameplay/link_session_manager.h"
#include "network/multiplayer_client.h"
#include "network/packet.h"
#include "save/save_manager.h"

namespace unboundmp::gameplay {

void LinkSessionManager::Initialize(network::MultiplayerClient* client, save::SaveManager* save_mgr) {
    client_ = client;
    save_mgr_ = save_mgr;
}

void LinkSessionManager::OnSessionAccepted(uint32_t session_id, save::LinkKind kind, uint64_t peer_account_id) {
    if (save_mgr_) {
        save_mgr_->BeginLinkSession(session_id, kind);
    }
    
    ActiveSession session;
    session.id = session_id;
    session.kind = kind;
    session.peer_account_id = peer_account_id;
    session.stream_seq = 0;
    active_session_ = session;
}

void LinkSessionManager::OnSessionData(uint32_t /*session_id*/, const std::vector<uint8_t>& /*data*/) {
    if (!active_session_) return;
    // Data forwarding would happen here
}

void LinkSessionManager::OnSessionEnded(uint32_t session_id, uint8_t reason) {
    if (!active_session_ || active_session_->id != session_id) return;
    
    if (save_mgr_) {
        bool completed = (reason == static_cast<uint8_t>(network::LinkSessionEndReason::kCompleted));
        save_mgr_->EndLinkSession(session_id, active_session_->kind, completed);
    }
    
    active_session_.reset();
}

void LinkSessionManager::SendLinkData(uint32_t session_id, const std::vector<uint8_t>& data) {
    if (!active_session_ || active_session_->id != session_id || !client_) return;
    
    network::LinkSessionDataPacket pkt_data;
    pkt_data.session_id = session_id;
    pkt_data.from_account_id = 0;
    pkt_data.payload = data;
    pkt_data.stream_seq = active_session_->stream_seq++;
    
    network::Packet pkt;
    pkt.type = network::PacketType::kLinkSessionData;
    pkt.payload = pkt_data.Serialize();
    client_->SendPacket(pkt);
}

bool LinkSessionManager::HasActiveSession() const {
    return active_session_.has_value();
}

std::optional<uint32_t> LinkSessionManager::GetActiveSessionId() const {
    if (active_session_) {
        return active_session_->id;
    }
    return std::nullopt;
}

save::LinkKind LinkSessionManager::GetActiveSessionKind() const {
    if (active_session_) {
        return active_session_->kind;
    }
    return save::LinkKind::kTrade;
}

} // namespace unboundmp::gameplay
