#include "persistence/story_service.h"

namespace unboundmp::persistence {

std::optional<memory::StoryData> StoryService::Get(pqxx::work& txn, uint64_t character_id) {
    auto result = txn.exec_prepared("get_story_blobs", character_id);
    if (result.empty()) return std::nullopt;
    
    memory::StoryData data;
    
    auto flags = result[0]["flags_blob"].as<pqxx::bytes>();
    const auto* f_data = reinterpret_cast<const uint8_t*>(flags.data());
    data.flags = std::vector<uint8_t>(f_data, f_data + flags.size());
    
    auto badges = result[0]["badges_blob"].as<pqxx::bytes>();
    const auto* b_data = reinterpret_cast<const uint8_t*>(badges.data());
    data.badges = std::vector<uint8_t>(b_data, b_data + badges.size());
    
    auto quests = result[0]["quests_blob"].as<pqxx::bytes>();
    const auto* q_data = reinterpret_cast<const uint8_t*>(quests.data());
    data.quests = std::vector<uint8_t>(q_data, q_data + quests.size());
    
    return data;
}

bool StoryService::Update(pqxx::work& txn, uint64_t character_id, const memory::StoryData& data) {
    auto f_ptr = reinterpret_cast<const std::byte*>(data.flags.data());
    pqxx::bytes flags_bytes(f_ptr, f_ptr + data.flags.size());
    
    auto b_ptr = reinterpret_cast<const std::byte*>(data.badges.data());
    pqxx::bytes badges_bytes(b_ptr, b_ptr + data.badges.size());
    
    auto q_ptr = reinterpret_cast<const std::byte*>(data.quests.data());
    pqxx::bytes quests_bytes(q_ptr, q_ptr + data.quests.size());
    
    auto result = txn.exec_prepared("upsert_story_blobs", character_id, flags_bytes, badges_bytes, quests_bytes);
    return result.affected_rows() > 0;
}

} // namespace unboundmp::persistence
