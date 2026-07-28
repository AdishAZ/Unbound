#pragma once

#include <cstdint>
#include <string>

namespace unboundmp::save {

// FNV-1a 64-bit over a byte buffer. Not cryptographic - this is a change
// detector for "did the save file's bytes move", not a security primitive,
// so a fast non-cryptographic hash with negligible accidental-collision
// probability for save-sized files (32-128KB SRAM/flash saves) is the right
// tool, and it avoids adding OpenSSL/libsodium as a build dependency.
uint64_t Fnv1a64(const uint8_t* data, size_t length);

// Hashes the file at `path`. Returns 0 with `out_existed = false` if the
// file does not exist; returns 0 with `out_existed = true` for a
// zero-length file (that's a legitimate, if unusual, state - not an
// error). Throws nothing; filesystem errors other than "does not exist"
// are surfaced by returning existed=false as well, since callers only ever
// use this for change-detection, not correctness-critical decisions.
uint64_t HashFile(const std::string& path, bool* out_existed, uint64_t* out_size_bytes);

}  // namespace unboundmp::save
