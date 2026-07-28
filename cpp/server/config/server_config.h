#pragma once

#include <string>

namespace unboundmp::server {

struct ServerConfig {
  std::string db_host = "localhost";
  uint16_t db_port = 5432;
  std::string db_user = "postgres";
  std::string db_password = "password";
  std::string db_name = "unboundmp";
  uint16_t listen_port = 4000;

  static ServerConfig Load(const std::string& path);
  
  std::string GetConnectionString() const;
};

}  // namespace unboundmp::server
