#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <pqxx/pqxx>

namespace unboundmp::persistence {

struct CharacterData {
    uint64_t id = 0;
    uint64_t account_id = 0;
    std::string name;
    std::string appearance;
    uint64_t play_time_seconds = 0;
    uint64_t created_at = 0;
    uint64_t last_login = 0;
    
    // Stage 8 additions
    uint32_t map_id = 0;
    float x = 0.0f;
    float y = 0.0f;
    uint8_t direction = 0;
    uint32_t money = 0;
};

class CharacterService {
public:
    static std::optional<CharacterData> GetByAccountId(pqxx::work& txn, uint64_t account_id);
    static bool Update(pqxx::work& txn, const CharacterData& character);
};

} // namespace unboundmp::persistence
