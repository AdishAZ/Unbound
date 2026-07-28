#pragma once

#include "models/account.h"
#include "models/character.h"
#include "network/connection.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <memory>
#include <optional>

namespace unboundmp::server {

struct Session {
  std::string token;
  Account account;
  std::optional<Character> active_character;
  network::Connection::Pointer connection;
  int64_t last_activity_time;
};

class SessionManager {
 public:
  SessionManager();
  
  std::string CreateSession(const Account& account, network::Connection::Pointer connection);
  
  bool AuthenticateConnection(const std::string& token, network::Connection::Pointer connection);
  void DisconnectSession(const std::string& token);
  void DisconnectByAccountId(uint64_t account_id);
  void DisconnectByConnectionId(uint32_t connection_id);
  
  std::shared_ptr<Session> GetSessionByToken(const std::string& token);
  std::shared_ptr<Session> GetSessionByConnectionId(uint32_t connection_id);
  
  void SetActiveCharacter(const std::string& token, const Character& character);
  void UpdateActivity(const std::string& token);
  
  void KickInactiveSessions(int64_t timeout_seconds);

 private:
  std::mutex sessions_mutex_;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_by_token_;
  std::unordered_map<uint64_t, std::string> account_to_token_;
  std::unordered_map<uint32_t, std::string> connection_to_token_;
};

}  // namespace unboundmp::server
