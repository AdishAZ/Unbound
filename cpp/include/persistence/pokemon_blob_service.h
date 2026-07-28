#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <pqxx/pqxx>

namespace unboundmp::persistence {

class PokemonBlobService {
public:
    static std::optional<std::vector<std::vector<uint8_t>>> GetParty(pqxx::work& txn, uint64_t character_id);
    static bool UpdateParty(pqxx::work& txn, uint64_t character_id, const std::vector<std::vector<uint8_t>>& slots);

    static std::optional<std::vector<uint8_t>> GetPc(pqxx::work& txn, uint64_t character_id);
    static bool UpdatePc(pqxx::work& txn, uint64_t character_id, const std::vector<uint8_t>& blob);
};

} // namespace unboundmp::persistence
