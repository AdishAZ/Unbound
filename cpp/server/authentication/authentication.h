#pragma once

#include <string>

namespace unboundmp::server {

class Authentication {
 public:
  static std::string HashPassword(const std::string& password);
  static bool VerifyPassword(const std::string& password, const std::string& hash);
  
  static std::string GenerateSessionToken();
};

}  // namespace unboundmp::server
