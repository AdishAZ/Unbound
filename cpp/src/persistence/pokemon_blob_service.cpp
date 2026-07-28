#include "persistence/pokemon_blob_service.h"

namespace unboundmp::persistence {

std::optional<std::vector<std::vector<uint8_t>>> PokemonBlobService::GetParty(pqxx::work& txn, uint64_t character_id) {
    auto result = txn.exec_prepared("get_party_blobs", character_id);
    if (result.empty()) return std::nullopt;
    
    std::vector<std::vector<uint8_t>> slots;
    slots.resize(result.size());
    for (const auto& row : result) {
        int16_t index = row["slot_index"].as<int16_t>();
        auto bytes = row["blob_data"].as<pqxx::bytes>();
        if (index >= 0 && static_cast<size_t>(index) < slots.size()) {
            const auto* ptr = reinterpret_cast<const uint8_t*>(bytes.data());
            slots[index] = std::vector<uint8_t>(ptr, ptr + bytes.size());
        }
    }
    return slots;
}

bool PokemonBlobService::UpdateParty(pqxx::work& txn, uint64_t character_id, const std::vector<std::vector<uint8_t>>& slots) {
    bool success = true;
    for (size_t i = 0; i < slots.size(); ++i) {
        const auto* ptr = reinterpret_cast<const std::byte*>(slots[i].data());
        pqxx::bytes bytes(ptr, ptr + slots[i].size());
        auto result = txn.exec_prepared("upsert_party_blob", character_id, static_cast<int16_t>(i), bytes);
        if (result.affected_rows() == 0) {
            success = false;
        }
    }
    return success;
}

std::optional<std::vector<uint8_t>> PokemonBlobService::GetPc(pqxx::work& txn, uint64_t character_id) {
    auto result = txn.exec_prepared("get_pc_blob", character_id);
    if (result.empty()) return std::nullopt;
    
    auto bytes = result[0]["blob_data"].as<pqxx::bytes>();
    const auto* ptr = reinterpret_cast<const uint8_t*>(bytes.data());
    return std::vector<uint8_t>(ptr, ptr + bytes.size());
}

bool PokemonBlobService::UpdatePc(pqxx::work& txn, uint64_t character_id, const std::vector<uint8_t>& blob) {
    const auto* ptr = reinterpret_cast<const std::byte*>(blob.data());
    pqxx::bytes bytes(ptr, ptr + blob.size());
    auto result = txn.exec_prepared("upsert_pc_blob", character_id, bytes);
    return result.affected_rows() > 0;
}

} // namespace unboundmp::persistence
