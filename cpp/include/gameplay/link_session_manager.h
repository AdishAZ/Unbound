#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "save/save_types.h"

namespace unboundmp::network { class MultiplayerClient; }
namespace unboundmp::save { class SaveManager; }

namespace unboundmp::gameplay {

class LinkSessionManager {
 public:
  void Initialize(network::MultiplayerClient* client, save::SaveManager* save_mgr);
  void OnSessionAccepted(uint32_t session_id, save::LinkKind kind, uint64_t peer_account_id);
  void OnSessionData(uint32_t session_id, const std::vector<uint8_t>& data);
  void OnSessionEnded(uint32_t session_id, uint8_t reason);
  void SendLinkData(uint32_t session_id, const std::vector<uint8_t>& data);
  
  bool HasActiveSession() const;
  std::optional<uint32_t> GetActiveSessionId() const;
  save::LinkKind GetActiveSessionKind() const;

 private:
  network::MultiplayerClient* client_ = nullptr;
  save::SaveManager* save_mgr_ = nullptr;
  
  struct ActiveSession { 
    uint32_t id; 
    save::LinkKind kind; 
    uint64_t peer_account_id; 
    uint64_t stream_seq = 0; 
  };
  std::optional<ActiveSession> active_session_;
};

} // namespace unboundmp::gameplay
