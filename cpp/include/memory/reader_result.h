#pragma once

#include <optional>
#include <string>
#include <utility>

namespace unboundmp::memory {

// Result of a reader operation: either a value, or an explanation of why
// there isn't one. The two failure modes readers distinguish:
//   - "not configured": the AddressTable has no entry for a symbol this
//     reader needs. This is expected until reverse engineering fills in
//     that symbol - see docs/REVERSE_ENGINEERING.md - and is not a crash,
//     just "this feature isn't available yet for this ROM build".
//   - anything else: a genuine problem (e.g. emulator not running).
template <typename T>
struct ReadResult {
  std::optional<T> value;
  std::string error;  // empty iff value.has_value()

  bool ok() const { return value.has_value(); }
  explicit operator bool() const { return ok(); }

  static ReadResult<T> Success(T v) { return ReadResult<T>{std::move(v), ""}; }
  static ReadResult<T> NotConfigured(const std::string& symbol_name) {
    return ReadResult<T>{std::nullopt,
                          "symbol '" + symbol_name +
                              "' is not configured in the address table - see "
                              "docs/REVERSE_ENGINEERING.md"};
  }
  static ReadResult<T> Failure(std::string message) {
    return ReadResult<T>{std::nullopt, std::move(message)};
  }
};

}  // namespace unboundmp::memory
