#pragma once

#include <cstdint>

namespace unboundmp::protocol {

// Network defaults. All overridable via server/client config; these are
// just the values used when nothing else is specified.
inline constexpr uint16_t kDefaultPort = 7777;
inline constexpr uint32_t kMaxPlayers = 32;

// Framing: every packet on the wire is [uint32 big-endian length][payload].
// Length is the size of the serialized Envelope, not including the 4-byte
// length prefix itself.
inline constexpr uint32_t kFrameLengthPrefixBytes = 4;
inline constexpr uint32_t kMaxFrameBytes = 64u * 1024u; // 64 KiB/packet cap

// Timing.
inline constexpr uint32_t kHeartbeatIntervalMs = 5000;
inline constexpr uint32_t kHeartbeatTimeoutMs = 15000;
inline constexpr uint32_t kWorldSnapshotIntervalMs = 10000;

// Player state broadcast rate. Position updates are sent no more often
// than this, client-side, to bound bandwidth; the server does not enforce
// this itself in this milestone (no rate limiting implemented yet).
inline constexpr uint32_t kPlayerStateSendIntervalMs = 100; // 10 Hz

}  // namespace unboundmp::protocol
