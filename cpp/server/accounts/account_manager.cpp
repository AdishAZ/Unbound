#include "accounts/account_manager.h"
#include "authentication/authentication.h"
#include <chrono>
#include <iostream>

namespace unboundmp::server {

AccountManager::AccountManager(std::shared_ptr<DatabasePool> db_pool)
    : db_pool_(std::move(db_pool)) {}

std::optional<Account> AccountManager::CreateAccount(const std::string& username, const std::string& password) {
  try {
    std::string hash = Authentication::HashPassword(password);
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) -> std::optional<Account> {
      pqxx::result res = txn.exec_prepared("create_account", username, hash, now);
      if (res.empty()) return std::nullopt;

      Account acc;
      acc.id = res[0][0].as<uint64_t>();
      acc.username = username;
      acc.password_hash = hash;
      acc.created_at = now;
      return acc;
    });
  } catch (const std::exception& e) {
    std::cerr << "Failed to create account: " << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<Account> AccountManager::Login(const std::string& username, const std::string& password) {
  try {
    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) -> std::optional<Account> {
      pqxx::result res = txn.exec_prepared("get_account_by_username", username);
      if (res.empty()) return std::nullopt;

      Account acc;
      acc.id = res[0]["id"].as<uint64_t>();
      acc.username = res[0]["username"].as<std::string>();
      acc.password_hash = res[0]["password_hash"].as<std::string>();
      acc.created_at = res[0]["created_at"].as<int64_t>();

      if (Authentication::VerifyPassword(password, acc.password_hash)) {
        return acc;
      }
      return std::nullopt;
    });
  } catch (const std::exception& e) {
    std::cerr << "Login failed: " << e.what() << std::endl;
    return std::nullopt;
  }
}

bool AccountManager::DeleteAccount(uint64_t account_id) {
  try {
    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
      pqxx::result res = txn.exec_prepared("delete_account", account_id);
      return res.affected_rows() > 0;
    });
  } catch (...) {
    return false;
  }
}

bool AccountManager::ChangePassword(uint64_t account_id, const std::string& old_password, const std::string& new_password) {
  try {
    return db_pool_->ExecuteTransaction([&](pqxx::work& txn) {
      pqxx::result res = txn.exec_prepared("get_account_by_id", account_id);
      if (res.empty()) return false;

      std::string hash = res[0]["password_hash"].as<std::string>();
      if (!Authentication::VerifyPassword(old_password, hash)) {
        return false;
      }

      std::string new_hash = Authentication::HashPassword(new_password);
      pqxx::result update_res = txn.exec_prepared("update_password", new_hash, account_id);
      return update_res.affected_rows() > 0;
    });
  } catch (...) {
    return false;
  }
}

}  // namespace unboundmp::server
