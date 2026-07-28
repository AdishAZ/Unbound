#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <pqxx/pqxx>
#include "memory/story_reader.h" // For StoryData (to reuse struct)

namespace unboundmp::persistence {

class StoryService {
public:
    static std::optional<memory::StoryData> Get(pqxx::work& txn, uint64_t character_id);
    static bool Update(pqxx::work& txn, uint64_t character_id, const memory::StoryData& data);
};

} // namespace unboundmp::persistence
