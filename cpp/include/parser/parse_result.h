#pragma once
#include <string>
#include <optional>
#include <cstdint>
#include <utility>

namespace unboundmp::parser {

template <typename T>
struct ParseResult {
    std::optional<T> value;
    std::string error;
    int64_t parsed_at_frame = 0;
    bool stale = false;

    bool ok() const { return value.has_value(); }

    static ParseResult<T> Success(T v, int64_t frame = 0) {
        return {std::make_optional(std::move(v)), "", frame, false};
    }

    static ParseResult<T> NotConfigured(const std::string& symbol_name) {
        return {std::nullopt, "Not configured: " + symbol_name, 0, false};
    }

    static ParseResult<T> Failure(std::string message) {
        return {std::nullopt, std::move(message), 0, false};
    }
};

} // namespace unboundmp::parser
