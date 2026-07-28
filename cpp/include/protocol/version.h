#pragma once

#include <cstdint>
#include <string_view>

namespace unboundmp::protocol {

// Bump per the rules in /proto/README.md#versioning whenever a
// wire-incompatible change is made to packets.proto. This value must match
// PROTOCOL_VERSION in server/internal/config/constants.go.
inline constexpr uint32_t kProtocolVersion = 1;

// Human-readable client build version. Purely informational — sent in
// ClientHello.client_version_string for logging/diagnostics, never parsed
// for compatibility decisions (protocol_version is what gates that).
inline constexpr std::string_view kClientVersionString = "0.1.0";

}  // namespace unboundmp::protocol
