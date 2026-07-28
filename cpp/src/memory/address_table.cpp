#include "memory/address_table.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace unboundmp::memory {

namespace {

std::string Trim(const std::string& s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::string ToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

bool ParseWidth(const std::string& token, ValueWidth& out_width) {
  const std::string t = ToLower(Trim(token));
  if (t == "u8") {
    out_width = ValueWidth::kU8;
    return true;
  }
  if (t == "u16") {
    out_width = ValueWidth::kU16;
    return true;
  }
  if (t == "u32") {
    out_width = ValueWidth::kU32;
    return true;
  }
  return false;
}

bool ParseAddress(const std::string& token, uint32_t& out_address) {
  const std::string t = Trim(token);
  if (t.empty()) {
    return false;
  }
  try {
    size_t consumed = 0;
    unsigned long value = std::stoul(t, &consumed, 0);  // base 0: honors "0x" prefix
    if (consumed != t.size()) {
      return false;
    }
    out_address = static_cast<uint32_t>(value);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

}  // namespace

std::optional<AddressTable> AddressTable::LoadFromFile(const std::string& path,
                                                         std::string& out_error) {
  std::ifstream file(path);
  if (!file.is_open()) {
    out_error = "Could not open address table file: " + path;
    return std::nullopt;
  }

  AddressTable table;
  std::string line;
  int line_number = 0;

  while (std::getline(file, line)) {
    ++line_number;
    const std::string trimmed = Trim(line);
    if (trimmed.empty() || trimmed[0] == '#') {
      continue;
    }

    const size_t eq_pos = trimmed.find('=');
    if (eq_pos == std::string::npos) {
      out_error = path + ":" + std::to_string(line_number) +
                   ": expected 'symbol_name = 0xADDRESS, width[, signed]'";
      return std::nullopt;
    }

    const std::string name = Trim(trimmed.substr(0, eq_pos));
    if (name.empty()) {
      out_error = path + ":" + std::to_string(line_number) + ": empty symbol name";
      return std::nullopt;
    }

    std::string rest = trimmed.substr(eq_pos + 1);
    std::vector<std::string> parts;
    {
      std::stringstream ss(rest);
      std::string part;
      while (std::getline(ss, part, ',')) {
        parts.push_back(Trim(part));
      }
    }

    if (parts.size() < 2 || parts.size() > 3) {
      out_error = path + ":" + std::to_string(line_number) +
                   ": expected 'symbol_name = 0xADDRESS, width[, signed]', got '" + trimmed + "'";
      return std::nullopt;
    }

    Symbol symbol;
    if (!ParseAddress(parts[0], symbol.address)) {
      out_error = path + ":" + std::to_string(line_number) + ": invalid address '" + parts[0] + "'";
      return std::nullopt;
    }
    if (!ParseWidth(parts[1], symbol.width)) {
      out_error =
          path + ":" + std::to_string(line_number) + ": invalid width '" + parts[1] + "' (expected u8/u16/u32)";
      return std::nullopt;
    }
    if (parts.size() == 3) {
      const std::string flag = ToLower(parts[2]);
      if (flag != "signed") {
        out_error = path + ":" + std::to_string(line_number) + ": unknown flag '" + parts[2] +
                     "' (only 'signed' is recognized)";
        return std::nullopt;
      }
      symbol.is_signed = true;
    }

    table.Set(name, symbol);
  }

  return table;
}

}  // namespace unboundmp::memory
