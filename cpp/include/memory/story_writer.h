#pragma once

#include <cstdint>
#include <vector>

#include "memory/address_table.h"
#include "memory/memory_api.h"
#include "memory/story_reader.h" // For StoryData

namespace unboundmp::memory {

class StoryWriter {
 public:
  StoryWriter(MemoryApi& memory, const AddressTable& addresses)
      : memory_(memory), addresses_(addresses) {}

  bool Write(const StoryData& data) {
    auto flags_symbol = addresses_.Get("story_flags_base");
    if (flags_symbol && !data.flags.empty()) {
        memory_.WriteBytes(flags_symbol->address, data.flags);
    }
    
    auto badges_symbol = addresses_.Get("badge_flags_base");
    if (badges_symbol && !data.badges.empty()) {
        memory_.WriteBytes(badges_symbol->address, data.badges);
    }
    
    auto quests_symbol = addresses_.Get("quest_flags_base");
    if (quests_symbol && !data.quests.empty()) {
        memory_.WriteBytes(quests_symbol->address, data.quests);
    }
    
    return true;
  }

 private:
  MemoryApi& memory_;
  const AddressTable& addresses_;
};

}  // namespace unboundmp::memory
