#include "world/player_manager.h"
#include "utils/logger.h"

namespace unboundmp::server::world {

std::shared_ptr<PlayerEntity> PlayerManager::RegisterPlayer(
    uint64_t account_id,
    uint64_t character_id,
    const std::string& session_token,
    const std::string& character_name) {

    std::lock_guard lock(mutex_);

    // If already registered, remove the old entry first.
    auto existing = players_by_account_.find(account_id);
    if (existing != players_by_account_.end()) {
        auto& old = existing->second;
        account_by_character_.erase(old->character_id);
        account_by_session_.erase(old->session_token);
        players_by_account_.erase(existing);
    }

    auto player = std::make_shared<PlayerEntity>();
    player->entity_id      = WorldEntity::NextEntityId();
    player->account_id     = account_id;
    player->character_id   = character_id;
    player->session_token  = session_token;
    player->character_name = character_name;

    players_by_account_[account_id]      = player;
    account_by_character_[character_id]  = account_id;
    account_by_session_[session_token]   = account_id;

    Logger::Info("PlayerManager: Registered player (account " +
                 std::to_string(account_id) +
                 ", character " + std::to_string(character_id) +
                 ", entity " + std::to_string(player->entity_id) + ")");
    return player;
}

bool PlayerManager::RemovePlayer(uint64_t account_id) {
    std::lock_guard lock(mutex_);

    auto it = players_by_account_.find(account_id);
    if (it == players_by_account_.end()) return false;

    auto& player = it->second;
    account_by_character_.erase(player->character_id);
    account_by_session_.erase(player->session_token);
    players_by_account_.erase(it);

    Logger::Info("PlayerManager: Removed player (account " +
                 std::to_string(account_id) + ")");
    return true;
}

std::shared_ptr<PlayerEntity> PlayerManager::FindPlayer(uint64_t account_id) const {
    std::lock_guard lock(mutex_);
    auto it = players_by_account_.find(account_id);
    if (it != players_by_account_.end()) return it->second;
    return nullptr;
}

std::shared_ptr<PlayerEntity> PlayerManager::FindByCharacter(uint64_t character_id) const {
    std::lock_guard lock(mutex_);
    auto it = account_by_character_.find(character_id);
    if (it == account_by_character_.end()) return nullptr;

    auto pit = players_by_account_.find(it->second);
    if (pit != players_by_account_.end()) return pit->second;
    return nullptr;
}

std::shared_ptr<PlayerEntity> PlayerManager::FindBySession(const std::string& session_token) const {
    std::lock_guard lock(mutex_);
    auto it = account_by_session_.find(session_token);
    if (it == account_by_session_.end()) return nullptr;

    auto pit = players_by_account_.find(it->second);
    if (pit != players_by_account_.end()) return pit->second;
    return nullptr;
}

std::vector<std::shared_ptr<PlayerEntity>> PlayerManager::GetAllPlayers() const {
    std::lock_guard lock(mutex_);
    std::vector<std::shared_ptr<PlayerEntity>> result;
    result.reserve(players_by_account_.size());
    for (const auto& [id, player] : players_by_account_) {
        result.push_back(player);
    }
    return result;
}

size_t PlayerManager::GetPlayerCount() const {
    std::lock_guard lock(mutex_);
    return players_by_account_.size();
}

}  // namespace unboundmp::server::world
