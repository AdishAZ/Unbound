# Protocol schema

`packets.proto` is the single source of truth for every packet exchanged
between a client and the multiplayer server. Nothing about map data, Pokemon
data, or move data lives here — see `LinkSessionData` in the schema for why
(trade/battle payloads are relayed as opaque bytes, not modeled).

Generated code is **not** committed. Generate it locally:

## C++

Handled automatically by CMake via `find_package(Protobuf REQUIRED)` and
`protobuf_generate_cpp()` — see `/cpp/CMakeLists.txt`. Just run the normal
CMake configure/build and the generated `packets.pb.cc` / `packets.pb.h`
will land in your build directory.

Requirements: `protobuf-compiler` and `libprotobuf-dev` (or equivalent)
installed on the build machine.

## Go

Requires `protoc` plus the Go plugin:

```sh
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
```

Then, from the repo root:

```sh
make proto-go
```

which runs:

```sh
protoc --go_out=. --go_opt=module=unbound-mp/server \
       --proto_path=proto \
       proto/packets.proto
```

This produces `server/internal/protocol/pb/packets.pb.go`, imported by
`internal/protocol/framing.go` and everything downstream of it.

## Versioning

`Envelope.protocol_version` is checked at handshake time
(`ClientHello` / `ServerHello`). See `PROTOCOL_VERSION` in
`cpp/include/protocol/version.h` and `server/internal/config/constants.go` —
these two values must always match a released protocol revision. Bump
`PROTOCOL_VERSION` whenever a change to `packets.proto` is not
wire-compatible with older clients (removing/renumbering a field,
changing a field's type, removing an enum value that's in use). Purely
additive changes (new optional field, new oneof case, new enum value) do
not require a bump.
