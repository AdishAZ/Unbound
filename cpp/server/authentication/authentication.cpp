#include "authentication/authentication.h"
#include <argon2.h>
#include <random>
#include <stdexcept>
#include <vector>

namespace unboundmp::server {

constexpr uint32_t t_cost = 2;            // 2-pass computation
constexpr uint32_t m_cost = (1 << 16);    // 64 mebibytes memory usage
constexpr uint32_t parallelism = 1;       // number of threads and lanes
constexpr size_t salt_len = 16;
constexpr size_t hash_len = 32;
constexpr size_t encoded_len = 108;       // length of base64 encoded argon2i string

std::string Authentication::HashPassword(const std::string& password) {
  std::vector<uint8_t> salt(salt_len);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint16_t> dis(0, 255);
  for (size_t i = 0; i < salt_len; ++i) {
    salt[i] = static_cast<uint8_t>(dis(gen));
  }

  char encoded[encoded_len];
  int result = argon2i_hash_encoded(t_cost, m_cost, parallelism,
                                    password.c_str(), password.length(),
                                    salt.data(), salt.size(),
                                    hash_len, encoded, encoded_len);
                                    
  if (result != ARGON2_OK) {
    throw std::runtime_error(std::string("Error hashing password: ") + argon2_error_message(result));
  }

  return std::string(encoded);
}

bool Authentication::VerifyPassword(const std::string& password, const std::string& hash) {
  int result = argon2i_verify(hash.c_str(), password.c_str(), password.length());
  return result == ARGON2_OK;
}

std::string Authentication::GenerateSessionToken() {
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, sizeof(charset) - 2);
  
  std::string token;
  token.reserve(32);
  for (int i = 0; i < 32; ++i) {
    token += charset[dis(gen)];
  }
  return token;
}

}  // namespace unboundmp::server
