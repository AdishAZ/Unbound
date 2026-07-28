This directory holds `packets.pb.go`, generated from `/proto/packets.proto`.
It is intentionally empty in source control - see `/proto/README.md` for the
`make proto-go` command that populates it. The rest of the server
(`internal/protocol`, `internal/connection`, `internal/player`, etc.) imports
`unbound-mp/server/internal/protocol/pb` and expects the generated
`Envelope`, `PlayerStateUpdate`, `AuthRequest`, etc. types to exist there
before the server will compile.
