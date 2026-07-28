#pragma once

#include "database/database.h"
#include "models/character.h"
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace unboundmp::server {

class CharacterManager {
 public:
  explicit CharacterManager(std::shared_ptr<DatabasePool> db_pool);

  std::optional<Character> CreateCharacter(uint64_t account_id, const std::string& name, const std::string& appearance);
  std::vector<Character> GetCharactersForAccount(uint64_t account_id);
  
bool UpdateCharacter(const Character& character);
  bool DeleteCharacter(uint64_t character_id);

  bool UpdateSaveStateBlob(uint64_t character_id, const std::vector<uint8_t>& blob);
  std::optional<std::vector<uint8_t>> GetSaveStateBlob(uint64_t character_id);

 private:
  std::shared_ptr<DatabasePool> db_pool_;
};

}  // namespace unboundmp::server
