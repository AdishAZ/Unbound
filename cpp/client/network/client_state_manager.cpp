#include "client_state_manager.h"

namespace unboundmp::client {

void ClientStateManager::SetState(ClientState state) {
  std::lock_guard<std::mutex> lock(mutex_);
  state_ = state;
}

ClientState ClientStateManager::GetState() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return state_;
}

void ClientStateManager::SetSessionToken(const std::string& token) {
  std::lock_guard<std::mutex> lock(mutex_);
  session_token_ = token;
}

std::string ClientStateManager::GetSessionToken() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return session_token_;
}

void ClientStateManager::SetAccountId(uint64_t account_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  account_id_ = account_id;
}

uint64_t ClientStateManager::GetAccountId() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return account_id_;
}

void ClientStateManager::SetCharacterId(uint64_t character_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  character_id_ = character_id;
}

uint64_t ClientStateManager::GetCharacterId() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return character_id_;
}

void ClientStateManager::Reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  state_ = ClientState::kDisconnected;
  session_token_.clear();
  account_id_ = 0;
  character_id_ = 0;
}


void ClientStateManager::SetSaveStateBlob(const std::vector<uint8_t>& blob) {
  std::lock_guard<std::mutex> lock(mutex_);
  save_state_blob_ = blob;
}

std::vector<uint8_t> ClientStateManager::GetSaveStateBlob() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return save_state_blob_;
}

}  // namespace unboundmp::client
