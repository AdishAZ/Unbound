#include "sessions/session_manager.h"
#include "authentication/authentication.h"
#include <chrono>

namespace unboundmp::server {

SessionManager::SessionManager() {}

std::string SessionManager::CreateSession(const Account& account, network::Connection::Pointer connection) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  
  // Enforce duplicate login prevention
  if (auto it = account_to_token_.find(account.id); it != account_to_token_.end()) {
    std::string old_token = it->second;
    if (auto s_it = sessions_by_token_.find(old_token); s_it != sessions_by_token_.end()) {
      if (s_it->second->connection) {
        s_it->second->connection->Disconnect();
      }
      connection_to_token_.erase(s_it->second->connection->GetId());
      sessions_by_token_.erase(s_it);
    }
    account_to_token_.erase(it);
  }

  std::string token = Authentication::GenerateSessionToken();
  auto session = std::make_shared<Session>();
  session->token = token;
  session->account = account;
  session->connection = connection;
  session->last_activity_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  
  sessions_by_token_[token] = session;
  account_to_token_[account.id] = token;
  if (connection) {
    connection_to_token_[connection->GetId()] = token;
  }
  
  return token;
}

bool SessionManager::AuthenticateConnection(const std::string& token, network::Connection::Pointer connection) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = sessions_by_token_.find(token);
  if (it == sessions_by_token_.end()) return false;
  
  auto session = it->second;
  if (session->connection && session->connection->GetId() != connection->GetId()) {
    connection_to_token_.erase(session->connection->GetId());
    session->connection->Disconnect();
  }
  
  session->connection = connection;
  connection_to_token_[connection->GetId()] = token;
  session->last_activity_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  
  return true;
}

void SessionManager::DisconnectSession(const std::string& token) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = sessions_by_token_.find(token);
  if (it != sessions_by_token_.end()) {
    if (it->second->connection) {
      connection_to_token_.erase(it->second->connection->GetId());
    }
    account_to_token_.erase(it->second->account.id);
    sessions_by_token_.erase(it);
  }
}

void SessionManager::DisconnectByAccountId(uint64_t account_id) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = account_to_token_.find(account_id);
  if (it != account_to_token_.end()) {
    std::string token = it->second;
    auto s_it = sessions_by_token_.find(token);
    if (s_it != sessions_by_token_.end()) {
      if (s_it->second->connection) {
        connection_to_token_.erase(s_it->second->connection->GetId());
        s_it->second->connection->Disconnect();
      }
      sessions_by_token_.erase(s_it);
    }
    account_to_token_.erase(it);
  }
}

void SessionManager::DisconnectByConnectionId(uint32_t connection_id) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = connection_to_token_.find(connection_id);
  if (it != connection_to_token_.end()) {
    std::string token = it->second;
    auto s_it = sessions_by_token_.find(token);
    if (s_it != sessions_by_token_.end()) {
      account_to_token_.erase(s_it->second->account.id);
      sessions_by_token_.erase(s_it);
    }
    connection_to_token_.erase(it);
  }
}

std::shared_ptr<Session> SessionManager::GetSessionByToken(const std::string& token) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = sessions_by_token_.find(token);
  if (it != sessions_by_token_.end()) {
    return it->second;
  }
  return nullptr;
}

std::shared_ptr<Session> SessionManager::GetSessionByConnectionId(uint32_t connection_id) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = connection_to_token_.find(connection_id);
  if (it != connection_to_token_.end()) {
    return sessions_by_token_[it->second];
  }
  return nullptr;
}

void SessionManager::SetActiveCharacter(const std::string& token, const Character& character) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = sessions_by_token_.find(token);
  if (it != sessions_by_token_.end()) {
    it->second->active_character = character;
  }
}

void SessionManager::UpdateActivity(const std::string& token) {
  std::lock_guard<std::mutex> lock(sessions_mutex_);
  auto it = sessions_by_token_.find(token);
  if (it != sessions_by_token_.end()) {
    it->second->last_activity_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  }
}

void SessionManager::KickInactiveSessions(int64_t timeout_seconds) {
  int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  
  std::vector<std::string> to_kick;
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (const auto& [token, session] : sessions_by_token_) {
      if (now - session->last_activity_time > timeout_seconds) {
        to_kick.push_back(token);
      }
    }
  }
  
  for (const auto& token : to_kick) {
    DisconnectSession(token);
  }
}

}  // namespace unboundmp::server
