#include "save/hashing.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace unboundmp::save {

namespace fs = std::filesystem;

uint64_t Fnv1a64(const uint8_t* data, size_t length) {
  constexpr uint64_t kOffsetBasis = 0xcbf29ce484222325ULL;
  constexpr uint64_t kPrime = 0x100000001b3ULL;

  uint64_t hash = kOffsetBasis;
  for (size_t i = 0; i < length; ++i) {
    hash ^= static_cast<uint64_t>(data[i]);
    hash *= kPrime;
  }
  return hash;
}

uint64_t HashFile(const std::string& path, bool* out_existed, uint64_t* out_size_bytes) {
  if (out_existed) *out_existed = false;
  if (out_size_bytes) *out_size_bytes = 0;

  std::error_code ec;
  if (!fs::exists(path, ec) || !fs::is_regular_file(path, ec)) {
    return 0;
  }
  if (out_existed) *out_existed = true;

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    // Exists but unreadable - treat like "does not exist" for hashing
    // purposes; SaveManager surfaces the underlying I/O failure separately
    // via ValidatePath()/SaveLoader before this is ever called.
    if (out_existed) *out_existed = false;
    return 0;
  }

  constexpr size_t kChunkSize = 64 * 1024;
  std::vector<uint8_t> buffer(kChunkSize);

  constexpr uint64_t kOffsetBasis = 0xcbf29ce484222325ULL;
  constexpr uint64_t kPrime = 0x100000001b3ULL;
  uint64_t hash = kOffsetBasis;
  uint64_t total_size = 0;

  while (file) {
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(kChunkSize));
    std::streamsize read = file.gcount();
    if (read <= 0) break;
    total_size += static_cast<uint64_t>(read);
    for (std::streamsize i = 0; i < read; ++i) {
      hash ^= static_cast<uint64_t>(buffer[static_cast<size_t>(i)]);
      hash *= kPrime;
    }
  }

  if (out_size_bytes) *out_size_bytes = total_size;
  return hash;
}

}  // namespace unboundmp::save
