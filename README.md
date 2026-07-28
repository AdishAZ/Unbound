# Pokemon Unbound multiplayer wrapper

A multiplayer layer built entirely **outside** the ROM. The ROM (Pokemon
Unbound) is never read as game data by the server, never patched, and never
has code injected into it. The wrapper embeds the mGBA core (a later
milestone) and adds networked presence, following, trading, and battling on
top of it.

In scope: seeing other players in the overworld, walking/running/biking/
surfing together, a following first-party Pokemon, trading, battling.

Explicitly out of scope: chat, guilds, shared story/NPCs, shared wild
encounters, shared inventory.

## Milestone status

This repository currently implements:

- **Shared networking protocol** (`/proto`, `/cpp/include/protocol`,
  `/cpp/src/protocol`) - every packet type, a protobuf schema, a
  length-prefixed serialization/framing layer, protocol constants, a
  version field with an upgrade policy, and a packet dispatcher.
- **Multiplayer server** (`/server`) - a Go TCP server: connection
  manager, player registry, heartbeats with timeout-based disconnect,
  simple username auth, per-connection session state, packet routing for
  presence/follower/link-session packets, and disconnect handling
  (including tearing down in-flight trade/battle link sessions).
- **Emulator abstraction** (`/cpp/include/emulator`, `/cpp/src/emulator`)
  - a backend-agnostic `IEmulatorCore` interface, ROM/save loaders that
    never modify the files they load, and a real libmgba-backed
    implementation (built only when `-DUNBOUNDMP_WITH_MGBA=ON`, so the
    rest of the client builds/tests without libmgba installed).
- **Memory reading** (`/cpp/include/memory`, `/cpp/src/memory`) - a
  reverse-engineered-address table (loaded from a config file, ships with
  zero addresses baked in), a thin `MemoryApi` wrapper over raw core
  reads, and per-feature readers (map, position, facing direction,
  movement state, party, follower, and OAM sprite attributes).
- **Client networking** (`/cpp/include/network`, `/cpp/src/network`) - a
  `NetworkManager` handling connect/handshake/auth, heartbeats +
  latency tracking, reconnection with backoff, and thread-safe
  send/receive queues.
- **Milestone 9 - Following Pokemon** (`/cpp/include/gameplay`,
  `/cpp/src/gameplay`) - a `FollowerManager` that turns
  `memory::FollowerState` + `protocol::FollowerUpdate` /
  `protocol::PlayerStateUpdate` into a per-player `FollowerVisualState`:
  species synchronization (with change-detection so an unchanging
  follower doesn't spam the network), a movement-trail-based offset so
  the follower trails its trainer by a configurable number of tiles
  (resetting cleanly across map transitions), direction logic (including
  a sensible "one tile behind current facing" spawn position before any
  trail history exists), and a walk-cycle/interpolation-progress
  animation clock keyed to the trainer's movement mode (walk/run/bike/
  surf). No memory reads, no network I/O, and no rendering of its own -
  it's the game-logic layer between those and a future
  SDL2/OpenGL rendering milestone.

- **Milestone 10 - 16:9 widescreen** (`/cpp/include/render`,
  `/cpp/src/render`) - the geometry foundation for playing in a widescreen
  window without stretching: `AspectRatio` classifies a window's shape
  (native 3:2 / 16:9 / ultrawide); `Camera` decides how many extra tile
  *columns* (never rows, so vertical FOV never changes) to reveal on each
  side to fill a wider window with more world instead of a distorted
  image, with an optional map-edge clamp so it never requests tiles past
  a map boundary; `ResolutionScaler` computes a uniform (always-equal-
  axes) scale factor in either pixel-perfect-integer or free-scale mode;
  `DynamicViewport` turns a window size + content size into the on-screen
  rect to draw into, recomputed on every resize; `OverlayLayout` resolves
  fixed HUD anchors and projects a remote player's tile offset into
  window pixel coordinates, so overlays stay correctly placed as the
  viewport and camera change size. Pure coordinate math - no SDL2/OpenGL,
  no window, no draw calls, and it doesn't yet know about actual on-
  screen sprites - it's the layer a future rendering milestone plugs into
  the same way `FollowerManager` (Milestone 9) is the game-logic layer
  that milestone will also consume.

- **Milestone 11 - Interaction detection** (`/cpp/include/interaction`,
  `/cpp/src/interaction`) - the game-logic layer that lets one player
  target another, built entirely on `protocol::PlayerStateUpdate` packets
  already flowing through the system since Milestone 9:
  `InteractionManager` tracks every player's latest (map, position,
  facing) and, for a given local player, lists every other tracked player
  within a configurable tile radius on the same map as an
  `InteractionCandidate` (distance + whether they're directly faced),
  sorted nearest-first with deterministic tie-breaking; `facing.h`/
  `distance.h` hold the underlying facing-detection (is the tile directly
  in front of a player occupied by a specific other player) and
  Manhattan-distance/same-map checks as small standalone functions;
  `PlayerSelector` resolves an actual target from those candidates -
  automatically preferring whoever is directly faced, falling back to
  nearest otherwise - or lets a player manually cycle through nearby
  candidates and remembers that manual selection until it's cleared or
  stops being valid (moved out of range, changed maps, disconnected). No
  memory reads, no network I/O, and no rendering - and it does not itself
  open a link session; it produces the `player_id` a future milestone
  would pass as `LinkSessionRequest.target_player_id` (see
  `proto/packets.proto`) once trading/battling are implemented.

Not yet implemented (later milestones): the actual SDL2/OpenGL rendering
milestone that consumes `/cpp/include/render` and Milestone 9's
`FollowerManager` to draw players/followers on screen, and trading/
battling themselves (Milestone 11 only detects and selects a target - it
doesn't open or drive a `LinkSessionRequest`).

- **Milestone 12 - Save synchronization** (`/cpp/include/save`,
  `/cpp/src/save`) - keeps each player's local save file correct around
  trade/battle link sessions: `SaveManager` orchestrates a `BackupManager`
  (rotating, tagged backups in a `backups/` directory next to the .sav
  file - `pre_trade`, `post_battle_completed`, etc., with automatic
  pruning) and a `ConflictDetector` (FNV-1a content hashing + mtime, to
  catch a save file changing when nothing in this process caused it - e.g.
  two client instances pointed at the same save). `BeginLinkSession`/
  `EndLinkSession` bracket a trade or battle: on completion, it verifies
  the save file's bytes actually changed and flags it if not, which is
  the concrete meaning of "trades and battles permanently affect saves"
  for a design where the emulator's own link-cable emulation - not this
  layer - is what mutates save data (see "Why trade/battle are a link
  session" below). Like `SaveLoader` before it, this treats save bytes as
  an opaque blob and never reads ROM data. Not yet wired to a real
  `LinkSessionEnd` handler or to `IEmulatorCore` - that's `NetworkManager`/
  `MgbaEmulatorCore` integration, future work.

- **Milestone 13 - Android support (foundation)** (`/android`,
  `/cpp/include/input`, `/cpp/include/platform`) - see `/android/README.md`
  for the full breakdown. In short: a Gradle/CMake Android project that
  builds the *same* `cpp/` libraries as desktop (not a fork of them) via a
  JNI bridge; `TouchInputMapper` (platform-agnostic multi-finger d-pad/
  button touch mapping, normalized coordinates, testable on desktop - see
  `touch_input_example`) and `AssetPathResolver` (where ROM cache/save/
  backup/config files live, sandboxed per-ROM on Android vs. ROM-sibling on
  desktop - see `asset_path_example`) are new reusable layers; `SaveManager`
  (Milestone 12) is reused as-is over JNI. Deliberately deferred: building
  `unboundmp_network`/`unboundmp_protocol` for Android (needs an
  Android-targeted Protobuf build; gated behind an opt-in CMake flag same
  as `UNBOUNDMP_WITH_MGBA`), an Android `MgbaEmulatorCore`, and any
  rendering - Android has no game screen to draw for the same reason
  desktop doesn't have one yet.

## Layout

```
proto/                      packets.proto - the wire schema (source of truth)
cpp/
  include/protocol/          constants.h, version.h, serialization.h, packet_dispatcher.h
  include/emulator/           IEmulatorCore, MgbaEmulatorCore, RomLoader, SaveLoader
  include/memory/             AddressTable, MemoryApi, and per-feature readers
  include/network/            NetworkManager, TcpSocket, heartbeat/latency/reconnect helpers
  include/gameplay/           FollowerManager and its supporting types (Milestone 9)
  include/render/              AspectRatio, Camera, DynamicViewport, ResolutionScaler,
                               OverlayLayout (Milestone 10)
  include/interaction/          InteractionManager, PlayerSelector, facing.h, distance.h
                               (Milestone 11)
  include/save/                 SaveManager, BackupManager, ConflictDetector, hashing.h
                               (Milestone 12)
  include/input/                 TouchInputMapper (Android-support milestone)
  include/platform/              AssetPathResolver (Android-support milestone)
  src/protocol/               serialization.cpp, packet_dispatcher.cpp
  src/emulator/                rom_loader.cpp, save_loader.cpp, mgba_emulator_core.cpp
  src/memory/                  address_table.cpp
  src/network/                 tcp_socket.cpp, network_manager.cpp
  src/gameplay/                 direction.cpp, follower_manager.cpp
  src/render/                   aspect_ratio.cpp, resolution_scaling.cpp, viewport.cpp,
                               camera.cpp, overlay_layout.cpp
  src/interaction/               distance.cpp, facing.cpp, interaction_manager.cpp,
                               player_selector.cpp
  src/save/                      hashing.cpp, conflict_detector.cpp, backup_manager.cpp,
                               save_manager.cpp
  src/input/                     touch_input_mapper.cpp
  src/platform/                  asset_path_resolver.cpp
  examples/                   one standalone smoke-test binary per layer (no ROM/server needed)
  CMakeLists.txt
android/                      Android app (Gradle + CMake/JNI) - see android/README.md
server/
  cmd/server/main.go          entrypoint
  internal/config/            constants.go (mirrors cpp/include/protocol/constants.h + version.h)
  internal/connection/         TCP accept loop, per-connection read/write goroutines
  internal/player/              session + registry of authenticated players
  internal/auth/                 username validation + reservation
  internal/network/              handshake, auth flow, packet dispatch/routing, link sessions, heartbeat
  internal/protocol/            Go framing layer + pb/ (generated code, not committed)
Makefile                      proto codegen + build targets
```

## Why trade/battle are a "link session" instead of modeled packets

Real GBA trades and battles run over the link cable's serial protocol,
which mGBA already emulates faithfully. Rather than reverse-engineering
Unbound's in-memory Pokemon/move/battle data structures so the server can
understand trade and battle packets, the server treats a trade or battle
as a **generic byte relay** between two clients (`LinkSessionData` in
`packets.proto`). Each client's own emulator core interprets the bytes
exactly as it would over a physical link cable. The server only manages
*session setup* (who's linking with whom, in what mode) and teardown - it
never touches the actual gameplay bytes. This also means the protocol
doesn't need to change if Unbound is updated.

## Building

See `proto/README.md` for one-time codegen setup (`protoc` +
`protoc-gen-go` for Go, `protobuf-compiler`/`libprotobuf-dev` for CMake).

```sh
make build-cpp      # cmake configure + build; builds protocol/emulator/memory/network/
                     # gameplay/render/interaction libraries plus one smoke-test binary
                     # per layer:
                     #   protocol_roundtrip_example, memory_probe_example,
                     #   network_manager_example, follower_manager_example,
                     #   widescreen_example, interaction_example
make build-server   # generates Go protobuf code, then builds the server binary
make run-server     # builds and runs the server on :7777 (override with -port)
```

By default the emulator layer builds without libmgba, using a fake core in
`memory_probe_example` so the memory/gameplay layers are testable on any
machine. Pass `-DUNBOUNDMP_WITH_MGBA=ON` to `cmake -S cpp -B build` to also
build the real libmgba-backed `MgbaEmulatorCore` (requires libmgba-dev).
