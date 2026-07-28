#pragma once

#include <string>
#include <mutex>
#include <cstdint>

namespace unboundmp::client {

enum class ClientState {
  kDisconnected,
  kConnecting,
  kConnected,
  kAuthenticating,
  kAuthenticated,
  kLoggedIn,
  kReconnecting
};

class ClientStateManager {
 public:
  ClientStateManager() = default;

  void SetState(ClientState state);
  ClientState GetState() const;

  void SetSessionToken(const std::string& token);
  std::string GetSessionToken() const;

  void SetAccountId(uint64_t account_id);
  uint64_t GetAccountId() const;

void SetCharacterId(uint64_t character_id);
  uint64_t GetCharacterId() const;
  
  void SetSaveStateBlob(const std::vector<uint8_t>& blob);
  std::vector<uint8_t> GetSaveStateBlob() const;

  void Reset();

 private:
  mutable std::mutex mutex_;
  ClientState state_ = ClientState::kDisconnected;
  std::string session_token_;
  uint64_t account_id_ = 0;
  uint64_t character_id_ = 0;
  std::vector<uint8_t> save_state_blob_;
};

}  // namespace unboundmp::client
