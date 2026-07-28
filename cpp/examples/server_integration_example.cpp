#include "config/server_config.h"
#include "database/database.h"
#include "authentication/authentication.h"
#include "accounts/account_manager.h"
#include "characters/character_manager.h"
#include "sessions/session_manager.h"
#include "utils/logger.h"

#include <iostream>
#include <memory>
#include <cassert>

using namespace unboundmp::server;

void Verify(bool condition, const std::string& message) {
  if (condition) {
    Logger::Info("[PASS] " + message);
  } else {
    Logger::Error("[FAIL] " + message);
    std::exit(1);
  }
}

int main() {
  Logger::Info("Starting Backend Integration Test...");

  ServerConfig config;
  
  std::shared_ptr<DatabasePool> db_pool;
  try {
    db_pool = std::make_shared<DatabasePool>(config.GetConnectionString(), 2);
    Verify(true, "Database connectivity and schema initialization");
  } catch (const std::exception& e) {
    Logger::Error(std::string("Database failed: ") + e.what());
    return 1;
  }

  AccountManager account_manager(db_pool);
  CharacterManager character_manager(db_pool);
  SessionManager session_manager;

  std::string test_user = "test_user_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
  std::string test_pass = "secure_password_123";

  // Test Account Creation
  auto acc = account_manager.CreateAccount(test_user, test_pass);
  Verify(acc.has_value(), "Create a test account");

  // Test Login and Password Hashing
  auto login_acc = account_manager.Login(test_user, test_pass);
  Verify(login_acc.has_value() && login_acc->id == acc->id, "Password hashing and login");

  auto bad_login = account_manager.Login(test_user, "wrong_password");
  Verify(!bad_login.has_value(), "Reject invalid password");

  // Test Session Creation
  std::string token1 = session_manager.CreateSession(*acc, nullptr);
  Verify(!token1.empty(), "Session creation");
  
  auto session1 = session_manager.GetSessionByToken(token1);
  Verify(session1 != nullptr && session1->account.id == acc->id, "Retrieve session by token");

  // Test Duplicate Login Prevention
  std::string token2 = session_manager.CreateSession(*acc, nullptr);
  Verify(!token2.empty(), "Second session created");
  Verify(session_manager.GetSessionByToken(token1) == nullptr, "Duplicate login prevention (old session kicked)");

  // Test Character Profile
  auto char_opt = character_manager.CreateCharacter(acc->id, "Ash", "Sprite1");
  Verify(char_opt.has_value(), "Create character profile");
  
  auto chars = character_manager.GetCharactersForAccount(acc->id);
  Verify(chars.size() == 1 && chars[0].name == "Ash", "Load character profile");
  
  chars[0].play_time_seconds = 3600;
  bool saved = character_manager.UpdateCharacter(chars[0]);
  Verify(saved, "Save character profile");
  
  auto updated_chars = character_manager.GetCharactersForAccount(acc->id);
  Verify(updated_chars[0].play_time_seconds == 3600, "Verify character save persistence");

  // Test Session Expiration
  session_manager.KickInactiveSessions(-1); // Kick immediately
  Verify(session_manager.GetSessionByToken(token2) == nullptr, "Session expiration");

  Logger::Info("Graceful shutdown and cleanup...");
  // Account manager deletion cleans up cascade (characters)
  bool deleted = account_manager.DeleteAccount(acc->id);
  Verify(deleted, "Account deletion (cleanup)");

  Logger::Info("All tests passed successfully.");
  return 0;
}
