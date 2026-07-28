#pragma once

#include "database/database.h"
#include "models/account.h"
#include <optional>
#include <string>
#include <memory>

namespace unboundmp::server {

class AccountManager {
 public:
  explicit AccountManager(std::shared_ptr<DatabasePool> db_pool);

  std::optional<Account> CreateAccount(const std::string& username, const std::string& password);
  std::optional<Account> Login(const std::string& username, const std::string& password);
  
  bool DeleteAccount(uint64_t account_id);
  bool ChangePassword(uint64_t account_id, const std::string& old_password, const std::string& new_password);

 private:
  std::shared_ptr<DatabasePool> db_pool_;
};

}  // namespace unboundmp::server
