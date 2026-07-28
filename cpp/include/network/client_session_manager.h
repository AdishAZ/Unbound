#pragma once
#include "network/client_player_session.h"
#include <memory>
#include <optional>
#include <string>

namespace unboundmp::network {

class ClientSessionManager {
public:
    static ClientSessionManager& GetInstance() {
        static ClientSessionManager instance;
        return instance;
    }

    void Initialize();

    void CreateSession(const std::string& token, uint64_t account_id);
    void DestroySession();
    
    std::optional<ClientPlayerSession> GetSession() const;
    ClientPlayerSession* GetMutableSession();
    
    void SetActiveCharacter(uint64_t character_id);
    void SetSessionState(SessionState state);
    
    bool IsAuthenticated() const;
    void UpdateHeartbeat();
    
    bool HasActiveSession() const { return active_session_ != nullptr; }

private:
    ClientSessionManager() = default;
    ~ClientSessionManager() = default;
    
    ClientSessionManager(const ClientSessionManager&) = delete;
    ClientSessionManager& operator=(const ClientSessionManager&) = delete;

    std::unique_ptr<ClientPlayerSession> active_session_;
};

} // namespace unboundmp::network
