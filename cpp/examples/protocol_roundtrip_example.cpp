// Standalone smoke test for the protocol foundation: build an Envelope,
// encode it to a byte frame, feed those bytes through FrameDecoder as if
// they'd arrived off a socket, and route the result through a
// PacketDispatcher. No networking or emulator code is involved — this
// exists purely to prove the protocol layer built in this milestone
// works end to end.

#include <cstdio>

#include "protocol/constants.h"
#include "protocol/packet_dispatcher.h"
#include "protocol/serialization.h"
#include "protocol/version.h"

using unboundmp::protocol::Envelope;
using unboundmp::protocol::EncodeFrame;
using unboundmp::protocol::FrameDecoder;
using unboundmp::protocol::PacketDispatcher;

int main() {
  // 1. Build a packet: a player state update.
  Envelope out_envelope;
  out_envelope.set_protocol_version(unboundmp::protocol::kProtocolVersion);
  out_envelope.set_sequence(1);

  auto* state = out_envelope.mutable_player_state_update();
  state->set_player_id(42);
  state->set_map_bank(1);
  state->set_map_number(2);
  state->set_x(10);
  state->set_y(15);
  state->set_facing(unboundmp::protocol::DIRECTION_DOWN);
  state->set_movement(unboundmp::protocol::MOVEMENT_MODE_RUN);
  state->set_is_moving(true);

  // 2. Encode it as it would be sent over the wire.
  std::vector<uint8_t> wire_bytes;
  if (!EncodeFrame(out_envelope, wire_bytes)) {
    std::fprintf(stderr, "failed to encode envelope\n");
    return 1;
  }
  std::printf("encoded %zu bytes\n", wire_bytes.size());

  // 3. Simulate receiving those bytes off a socket, possibly split across
  //    multiple reads.
  FrameDecoder decoder;
  const size_t half = wire_bytes.size() / 2;
  decoder.Feed(wire_bytes.data(), half);
  decoder.Feed(wire_bytes.data() + half, wire_bytes.size() - half);

  auto decoded = decoder.TryExtractNext();
  if (!decoded.has_value()) {
    std::fprintf(stderr, "failed to decode envelope (corrupted=%d)\n", decoder.IsCorrupted());
    return 1;
  }

  // 4. Route it through the dispatcher.
  PacketDispatcher dispatcher;
  dispatcher.RegisterHandler(Envelope::kPlayerStateUpdate, [](const Envelope& envelope) {
    const auto& s = envelope.player_state_update();
    std::printf("dispatched PlayerStateUpdate for player_id=%u at (%d,%d)\n",
                s.player_id(), s.x(), s.y());
  });
  dispatcher.RegisterUnhandled([](const Envelope&) {
    std::printf("unhandled packet type\n");
  });

  dispatcher.Dispatch(*decoded);
  return 0;
}
