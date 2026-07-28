#pragma once

#include <cstdint>
#include <string>

namespace unboundmp::server {

struct Account {
  uint64_t id = 0;
  std::string username;
  std::string password_hash;
  int64_t created_at = 0;
};

}  // namespace unboundmp::server
