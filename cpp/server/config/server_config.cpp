#include "config/server_config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace unboundmp::server {

ServerConfig ServerConfig::Load(const std::string& path) {
  ServerConfig config;
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Warning: Could not open config file " << path << ". Using defaults." << std::endl;
    return config;
  }
  
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;
    
    auto delim = line.find('=');
    if (delim == std::string::npos) continue;
    
    std::string key = line.substr(0, delim);
    std::string value = line.substr(delim + 1);
    
    // Trim
    key.erase(key.find_last_not_of(" \n\r\t") + 1);
    value.erase(value.find_last_not_of(" \n\r\t") + 1);
    
    if (key == "DB_HOST") config.db_host = value;
    else if (key == "DB_PORT") config.db_port = std::stoi(value);
    else if (key == "DB_USER") config.db_user = value;
    else if (key == "DB_PASSWORD") config.db_password = value;
    else if (key == "DB_NAME") config.db_name = value;
    else if (key == "LISTEN_PORT") config.listen_port = std::stoi(value);
  }
  
  return config;
}

std::string ServerConfig::GetConnectionString() const {
  std::ostringstream oss;
  oss << "postgresql://" << db_user << ":" << db_password << "@" << db_host << ":" << db_port << "/" << db_name << "?sslmode=disable";
  return oss.str();
}

}  // namespace unboundmp::server
