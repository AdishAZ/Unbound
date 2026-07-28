#pragma once

#include "database/database.h"
#include "models/character.h"
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace unboundmp::server {

class CharacterRepository {
 public:
  explicit CharacterRepository(std::shared_ptr<DatabasePool> db_pool);

  std::optional<Character> CreateCharacter(uint64_t account_id, const std::string& name, const std::string& appearance);
  std::optional<Character> LoadCharacter(uint64_t character_id);
  std::vector<Character> LoadCharactersForAccount(uint64_t account_id);
  
  bool UpdateCharacter(const Character& character);
  bool DeleteCharacter(uint64_t account_id, uint64_t character_id);
  
  bool CharacterExists(uint64_t character_id);
  bool CharacterNameExists(const std::string& name);
  bool RenameCharacter(uint64_t character_id, const std::string& new_name);

 private:
  bool IsNameValid(const std::string& name) const;
  
  std::shared_ptr<DatabasePool> db_pool_;
};

}  // namespace unboundmp::server
