#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <pqxx/pqxx>

namespace unboundmp::persistence {

class InventoryService {
public:
    static std::optional<std::vector<uint8_t>> Get(pqxx::work& txn, uint64_t character_id);
    static bool Update(pqxx::work& txn, uint64_t character_id, const std::vector<uint8_t>& blob);
};

} // namespace unboundmp::persistence
