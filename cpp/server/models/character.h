#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace unboundmp::server {

class Character {
 public:
  uint64_t id = 0;
  uint64_t account_id = 0;
  std::string name;
  std::string appearance;
  int64_t play_time_seconds = 0;
  int64_t created_at = 0;
  int64_t last_login = 0;
  
  // Location
  uint32_t map_id = 0;
  float x = 0.0f;
  float y = 0.0f;
  uint8_t direction = 0;
  
  // Resources
  uint64_t money = 0;

  // Persistence Blobs
  std::vector<uint8_t> save_state_blob;
  std::vector<uint8_t> inventory_blob;
  std::vector<std::vector<uint8_t>> party_slots;
  std::vector<uint8_t> pc_blob;
  std::vector<uint8_t> story_flags;
  std::vector<uint8_t> story_badges;
  std::vector<uint8_t> story_quests;
};

}  // namespace unboundmp::server
