#include "persistence/character_service.h"

namespace unboundmp::persistence {

std::optional<CharacterData> CharacterService::GetByAccountId(pqxx::work& txn, uint64_t account_id) {
    auto result = txn.exec_prepared("get_character_by_account", account_id);
    if (result.empty()) return std::nullopt;
    
    const auto& row = result[0];
    CharacterData data;
    data.id = row["id"].as<uint64_t>();
    data.account_id = row["account_id"].as<uint64_t>();
    data.name = row["name"].as<std::string>();
    data.appearance = row["appearance"].as<std::string>();
    data.play_time_seconds = static_cast<uint64_t>(row["play_time_seconds"].as<int64_t>());
    data.created_at = static_cast<uint64_t>(row["created_at"].as<int64_t>());
    data.last_login = static_cast<uint64_t>(row["last_login"].as<int64_t>());
    data.map_id = row["map_id"].as<uint32_t>();
    data.x = row["x"].as<float>();
    data.y = row["y"].as<float>();
    data.direction = static_cast<uint8_t>(row["direction"].as<int>());
    data.money = static_cast<uint32_t>(row["money"].as<int64_t>());
    return data;
}

bool CharacterService::Update(pqxx::work& txn, const CharacterData& character) {
    auto result = txn.exec_prepared(
        "update_character", 
        character.name,
        character.appearance,
        character.play_time_seconds,
        character.last_login,
        character.map_id,
        character.x,
        character.y,
        static_cast<uint16_t>(character.direction),
        character.money,
        character.id
    );
    return result.affected_rows() > 0;
}

} // namespace unboundmp::persistence
