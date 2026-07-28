#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "memory/memory_api.h"

namespace unboundmp::memory {

// One reverse-engineered memory location: an address plus how wide the
// field there is, and whether it should be treated as signed. Everything
// about *what* the value means (map bank vs. a follower species ID vs.
// whatever) is up to whichever reader class looks it up - Symbol itself is
// just "a place in memory and how to read it".
struct Symbol {
  uint32_t address = 0;
  ValueWidth width = ValueWidth::kU8;
  bool is_signed = false;
};

// A named table of Symbols, loaded from an external config file at
// runtime. This class intentionally ships with zero addresses baked in -
// see docs/REVERSE_ENGINEERING.md for how to find Unbound's actual
// addresses, and config/address_table.example.cfg for the file format and
// the full list of symbol names every reader in this codebase looks for.
//
// Readers ask AddressTable::Get("symbol_name") and get back
// std::nullopt if that symbol hasn't been configured yet, rather than a
// guessed/zero address that would silently read garbage. Callers are
// expected to check IsConfigured() (or handle nullopt) and fail loudly -
// see each reader's `Read()` method for the pattern.
class AddressTable {
 public:
  // Parses a simple line-oriented config file:
  //   # comment
  //   symbol_name = 0xADDRESS, width[, signed]
  // width is one of: u8, u16, u32
  // "signed" is a literal optional third token; its absence means
  // unsigned. Blank lines and lines starting with '#' are ignored.
  // Unknown/malformed lines cause LoadFromFile to fail with a message
  // naming the offending line, rather than silently skipping it - a typo
  // in an address is exactly the kind of mistake this format should catch
  // loudly instead of quietly loading a wrong address.
  static std::optional<AddressTable> LoadFromFile(const std::string& path, std::string& out_error);

  bool IsConfigured(const std::string& symbol_name) const {
    return symbols_.find(symbol_name) != symbols_.end();
  }

  std::optional<Symbol> Get(const std::string& symbol_name) const {
    auto it = symbols_.find(symbol_name);
    if (it == symbols_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  void Set(const std::string& symbol_name, Symbol symbol) {
    symbols_[symbol_name] = symbol;
  }

  size_t ConfiguredCount() const { return symbols_.size(); }

 private:
  std::unordered_map<std::string, Symbol> symbols_;
};

}  // namespace unboundmp::memory
