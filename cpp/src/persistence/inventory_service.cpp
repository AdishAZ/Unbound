#include "persistence/inventory_service.h"

namespace unboundmp::persistence {

std::optional<std::vector<uint8_t>> InventoryService::Get(pqxx::work& txn, uint64_t character_id) {
    auto result = txn.exec_prepared("get_inventory_blob", character_id);
    if (result.empty()) return std::nullopt;
    
    auto field = result[0]["blob_data"];
    auto bytes = field.as<pqxx::bytes>();
    const auto* ptr = reinterpret_cast<const uint8_t*>(bytes.data());
    std::vector<uint8_t> data(ptr, ptr + bytes.size());
    return data;
}

bool InventoryService::Update(pqxx::work& txn, uint64_t character_id, const std::vector<uint8_t>& blob) {
    const auto* ptr = reinterpret_cast<const std::byte*>(blob.data());
    pqxx::bytes bytes(ptr, ptr + blob.size());
    auto result = txn.exec_prepared("upsert_inventory_blob", character_id, bytes);
    return result.affected_rows() > 0;
}

} // namespace unboundmp::persistence
