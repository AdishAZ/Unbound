#include "characters/character_manager.h"
#include <chrono>
#include <iostream>

namespace unboundmp::server {

CharacterManager::CharacterManager(std::shared_ptr<DatabasePool> db_pool)
    : db_pool_(std::move(db_pool)) {}

std::optional<Character> CharacterManager::CreateCharacter(uint64_t account_id, const std::string& name, const std::string& appearance) {
  try {
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) -> std::optional<Character> {
      pqxx::result res = txn.exec_prepared("create_character", account_id, name, appearance, 0, now, now);
      if (res.empty()) return std::nullopt;

      Character c;
      c.id = res[0][0].as<uint64_t>();
      c.account_id = account_id;
      c.name = name;
      c.appearance = appearance;
      c.play_time_seconds = 0;
      c.created_at = now;
      c.last_login = now;
      return c;
    });
  } catch (const std::exception& e) {
    std::cerr << "Failed to create character: " << e.what() << std::endl;
    return std::nullopt;
  }
}

std::vector<Character> CharacterManager::GetCharactersForAccount(uint64_t account_id) {
  std::vector<Character> characters;
  try {
    db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
      pqxx::result res = txn.exec_prepared("get_character_by_account", account_id);
      for (const auto& row : res) {
        Character c;
        c.id = row["id"].as<uint64_t>();
        c.account_id = row["account_id"].as<uint64_t>();
        c.name = row["name"].as<std::string>();
        c.appearance = row["appearance"].as<std::string>();
        c.play_time_seconds = row["play_time_seconds"].as<int64_t>();
        c.created_at = row["created_at"].as<int64_t>();
        c.last_login = row["last_login"].as<int64_t>();
        characters.push_back(c);
      }
      return true;
    });
  } catch (const std::exception& e) {
    std::cerr << "Failed to fetch characters: " << e.what() << std::endl;
  }
  return characters;
}

bool CharacterManager::UpdateCharacter(const Character& character) {
  try {
    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
      pqxx::result res = txn.exec_prepared("update_character", character.name, character.appearance, character.play_time_seconds, character.last_login, character.id);
      return res.affected_rows() > 0;
    });
  } catch (...) {
    return false;
  }
}

bool CharacterManager::DeleteCharacter(uint64_t character_id) {
  try {
    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
      pqxx::result res = txn.exec_prepared("delete_character", character_id);
      return res.affected_rows() > 0;
    });
  } catch (...) {
    return false;
  }
}


bool CharacterManager::UpdateSaveStateBlob(uint64_t character_id, const std::vector<uint8_t>& blob) {
    try {
        auto conn = db_pool_->GetConnection();
        pqxx::work txn(*conn);
        
        std::basic_string_view<std::byte> bin_data(reinterpret_cast<const std::byte*>(blob.data()), blob.size());
        txn.exec_params(
            "UPDATE characters SET save_state_blob =  WHERE id = ",
            bin_data,
            character_id
        );
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to update save state blob: " + std::string(e.what()));
        return false;
    }
}

std::optional<std::vector<uint8_t>> CharacterManager::GetSaveStateBlob(uint64_t character_id) {
    try {
        auto conn = db_pool_->GetConnection();
        pqxx::work txn(*conn);
        
        auto result = txn.exec_params(
            "SELECT save_state_blob FROM characters WHERE id =  AND save_state_blob IS NOT NULL",
            character_id
        );
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        auto field = result[0][0];
        if (field.is_null()) return std::nullopt;
        
        auto bin_data = field.as<std::basic_string<std::byte>>();
        return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(bin_data.data()), reinterpret_cast<const uint8_t*>(bin_data.data()) + bin_data.size());
    } catch (const std::exception& e) {
        Logger::Error("Failed to get save state blob: " + std::string(e.what()));
        return std::nullopt;
    }
}

}  // namespace unboundmp::server
