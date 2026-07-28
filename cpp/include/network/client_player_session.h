#pragma once
#include <cstdint>
#include <string>

namespace unboundmp::network {

enum class SessionState {
    kConnecting,
    kAuthenticated,
    kCharacterSelected,
    kLoading,
    kInGame,
    kDisconnected
};

struct ClientPlayerSession {
    std::string session_token;
    uint64_t account_id = 0;
    uint64_t character_id = 0;
    
    SessionState state = SessionState::kDisconnected;
    
    int64_t login_timestamp = 0;
    int64_t last_heartbeat_timestamp = 0;
};

} // namespace unboundmp::network
