content_h = '''#pragma once

#include "database/database.h"
#include "models/character.h"
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace unboundmp::server {

class CharacterRepository {
 public:
  explicit CharacterRepository(std::shared_ptr<DatabasePool> db_pool);

  std::optional<Character> CreateCharacter(uint64_t account_id, const std::string& name, const std::string& appearance);
  std::optional<Character> LoadCharacter(uint64_t character_id);
  std::vector<Character> LoadCharactersForAccount(uint64_t account_id);
  
  bool UpdateCharacter(const Character& character);
  bool DeleteCharacter(uint64_t account_id, uint64_t character_id);
  
  bool CharacterExists(uint64_t character_id);
  bool CharacterNameExists(const std::string& name);
  bool RenameCharacter(uint64_t character_id, const std::string& new_name);

 private:
  bool IsNameValid(const std::string& name) const;
  
  std::shared_ptr<DatabasePool> db_pool_;
};

}  // namespace unboundmp::server
'''

content_cpp = '''#include "database/character_repository.h"
#include "utils/logger.h"
#include <chrono>
#include <iostream>
#include <cctype>
#include <algorithm>

namespace unboundmp::server {

CharacterRepository::CharacterRepository(std::shared_ptr<DatabasePool> db_pool)
    : db_pool_(std::move(db_pool)) {}

bool CharacterRepository::IsNameValid(const std::string& name) const {
    if (name.length() < 3 || name.length() > 16) {
        return false;
    }
    
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c){ return std::tolower(c); });
    
    std::vector<std::string> reserved = {"admin", "gm", "moderator", "system"};
    for (const auto& r : reserved) {
        if (lower_name == r) return false;
    }

    bool last_was_space = false;
    for (char c : name) {
        if (!std::isalnum(c) && c != ' ' && c != '-' && c != '_') {
            return false;
        }
        if (c == ' ') {
            if (last_was_space) return false;
            last_was_space = true;
        } else {
            last_was_space = false;
        }
    }
    
    if (name.front() == ' ' || name.back() == ' ') return false;

    return true;
}

std::optional<Character> CharacterRepository::CreateCharacter(uint64_t account_id, const std::string& name, const std::string& appearance) {
    if (!IsNameValid(name)) {
        Logger::Warn("Failed to create character: Invalid name format '" + name + "'");
        return std::nullopt;
    }

    try {
        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) -> std::optional<Character> {
            pqxx::result check_res = txn.exec_prepared("check_character_name", name);
            if (!check_res.empty()) {
                Logger::Warn("Failed to create character: Name '" + name + "' is already taken");
                return std::nullopt;
            }

            pqxx::result res = txn.exec_prepared("create_character", account_id, name, appearance, 0, now, now);
            if (res.empty()) return std::nullopt;

            uint64_t character_id = res[0][0].as<uint64_t>();
            
            // Initialize empty blobs
            std::vector<uint8_t> empty_blob;
            std::string empty_str = "";
            pqxx::binarystring empty_bin(empty_str.data(), empty_str.size());
            
            txn.exec_prepared("upsert_inventory_blob", character_id, empty_bin);
            txn.exec_prepared("upsert_pc_blob", character_id, empty_bin);
            txn.exec_prepared("upsert_story_blobs", character_id, empty_bin, empty_bin, empty_bin);
            
            for (int i = 0; i < 6; i++) {
                 txn.exec_prepared("upsert_party_blob", character_id, i, empty_bin);
            }

            Character c;
            c.id = character_id;
            c.account_id = account_id;
            c.name = name;
            c.appearance = appearance;
            c.play_time_seconds = 0;
            c.created_at = now;
            c.last_login = now;
            c.map_id = 0;
            c.x = 0;
            c.y = 0;
            c.direction = 0;
            c.money = 0;
            
            Logger::Info("Created character '" + name + "' for account " + std::to_string(account_id));
            return c;
        });
    } catch (const std::exception& e) {
        Logger::Error("Failed to create character: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<Character> CharacterRepository::LoadCharacter(uint64_t character_id) {
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) -> std::optional<Character> {
            pqxx::result res = txn.exec_prepared("get_character_by_id", character_id);
            if (res.empty()) return std::nullopt;
            
            auto row = res[0];
            Character c;
            c.id = row["id"].as<uint64_t>();
            c.account_id = row["account_id"].as<uint64_t>();
            c.name = row["name"].as<std::string>();
            c.appearance = row["appearance"].as<std::string>();
            c.play_time_seconds = row["play_time_seconds"].as<int64_t>();
            c.created_at = row["created_at"].as<int64_t>();
            c.last_login = row["last_login"].as<int64_t>();
            c.map_id = row["map_id"].as<uint32_t>();
            c.x = row["x"].as<float>();
            c.y = row["y"].as<float>();
            c.direction = row["direction"].as<uint8_t>();
            c.money = row["money"].as<uint64_t>();
            return c;
        });
    } catch (const std::exception& e) {
        Logger::Error("Failed to fetch character: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Character> CharacterRepository::LoadCharactersForAccount(uint64_t account_id) {
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
                c.map_id = row["map_id"].as<uint32_t>();
                c.x = row["x"].as<float>();
                c.y = row["y"].as<float>();
                c.direction = row["direction"].as<uint8_t>();
                c.money = row["money"].as<uint64_t>();
                characters.push_back(c);
            }
            return true;
        });
    } catch (const std::exception& e) {
        Logger::Error("Failed to fetch characters for account: " + std::string(e.what()));
    }
    return characters;
}

bool CharacterRepository::UpdateCharacter(const Character& character) {
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::result res = txn.exec_prepared("update_character", character.name, character.appearance, character.play_time_seconds, character.last_login, character.map_id, character.x, character.y, character.direction, character.money, character.id);
            return res.affected_rows() > 0;
        });
    } catch (const std::exception& e) {
        Logger::Error("Failed to update character: " + std::string(e.what()));
        return false;
    }
}

bool CharacterRepository::DeleteCharacter(uint64_t account_id, uint64_t character_id) {
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::result res = txn.exec_prepared("delete_character_safe", character_id, account_id);
            if (res.affected_rows() > 0) {
                Logger::Info("Deleted character " + std::to_string(character_id) + " for account " + std::to_string(account_id));
                return true;
            } else {
                Logger::Warn("Failed to delete character " + std::to_string(character_id) + " for account " + std::to_string(account_id) + " (not found or not owned)");
                return false;
            }
        });
    } catch (const std::exception& e) {
        Logger::Error("Failed to delete character: " + std::string(e.what()));
        return false;
    }
}

bool CharacterRepository::CharacterExists(uint64_t character_id) {
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::result res = txn.exec_prepared("check_character_exists", character_id);
            return !res.empty();
        });
    } catch (...) {
        return false;
    }
}

bool CharacterRepository::CharacterNameExists(const std::string& name) {
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::result res = txn.exec_prepared("check_character_name", name);
            return !res.empty();
        });
    } catch (...) {
        return false;
    }
}

bool CharacterRepository::RenameCharacter(uint64_t character_id, const std::string& new_name) {
    if (!IsNameValid(new_name)) return false;
    
    try {
        return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
            pqxx::result check_res = txn.exec_prepared("check_character_name", new_name);
            if (!check_res.empty()) return false;
            
            pqxx::result res = txn.exec_prepared("rename_character", new_name, character_id);
            return res.affected_rows() > 0;
        });
    } catch (...) {
        return false;
    }
}

}  // namespace unboundmp::server
'''

with open('d:/Unbound/pokemon/cpp/server/database/character_repository.h', 'w') as f:
    f.write(content_h)
    
with open('d:/Unbound/pokemon/cpp/server/database/character_repository.cpp', 'w') as f:
    f.write(content_cpp)
