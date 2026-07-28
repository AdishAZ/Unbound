#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/reader_result.h"

namespace unboundmp::memory {

struct StoryData {
  std::vector<uint8_t> flags;
  std::vector<uint8_t> badges;
  std::vector<uint8_t> quests;
};

struct StoryLayout {
  uint32_t flags_size_bytes = 300; // Gen 3 standard (0x12C bytes)
  uint32_t badges_size_bytes = 4;
  uint32_t quests_size_bytes = 16;
};

class StoryReader {
 public:
  StoryReader(const MemoryApi& memory, const AddressTable& addresses, StoryLayout layout = {})
      : memory_(memory), addresses_(addresses), layout_(layout) {}

  ReadResult<StoryData> Read() const {
    StoryData data;
    
    auto flags_symbol = addresses_.Get("story_flags_base");
    if (flags_symbol) {
        data.flags = memory_.ReadBytes(flags_symbol->address, layout_.flags_size_bytes);
    }
    
    auto badges_symbol = addresses_.Get("badge_flags_base");
    if (badges_symbol) {
        data.badges = memory_.ReadBytes(badges_symbol->address, layout_.badges_size_bytes);
    }
    
    auto quests_symbol = addresses_.Get("quest_flags_base");
    if (quests_symbol) {
        data.quests = memory_.ReadBytes(quests_symbol->address, layout_.quests_size_bytes);
    }
    
    // We consider it a success even if some are missing, as long as it's partial or none
    return ReadResult<StoryData>::Success(std::move(data));
  }

 private:
  const MemoryApi& memory_;
  const AddressTable& addresses_;
  StoryLayout layout_;
};

}  // namespace unboundmp::memory
