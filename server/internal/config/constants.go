// Package config holds every tunable constant for the multiplayer server.
// These values mirror cpp/include/protocol/constants.h and version.h on the
// client side - if you change a value here that affects wire compatibility,
// change it there too, and read proto/README.md#versioning before touching
// ProtocolVersion specifically.
package config

import "time"

const (
	// ProtocolVersion must match unboundmp::protocol::kProtocolVersion in
	// cpp/include/protocol/version.h.
	ProtocolVersion uint32 = 1

	ServerVersionString = "0.1.0"

	DefaultPort = 7777
	MaxPlayers  = 32

	// Framing: [uint32 big-endian length][payload]. Must match
	// kFrameLengthPrefixBytes / kMaxFrameBytes in cpp/include/protocol/constants.h.
	FrameLengthPrefixBytes = 4
	MaxFrameBytes          = 64 * 1024

	// MaxUsernameLength bounds AuthRequest.username. Enforced server-side;
	// see internal/auth.
	MaxUsernameLength = 16
	MinUsernameLength = 3
)

const (
	HeartbeatInterval     = 5 * time.Second
	HeartbeatTimeout      = 15 * time.Second
	WorldSnapshotInterval = 10 * time.Second

	// HandshakeTimeout bounds how long a freshly-accepted TCP connection has
	// to send ClientHello + AuthRequest before the server closes it.
	HandshakeTimeout = 10 * time.Second

	// WriteQueueSize is the number of outbound Envelopes buffered per
	// connection before the writer goroutine is considered backed up.
	// A connection whose write queue is full is disconnected rather than
	// letting a slow client apply backpressure to the whole server.
	WriteQueueSize = 128
)
