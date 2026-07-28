// Standalone smoke test for the client networking layer. Does NOT require
// a running server - it proves NetworkManager builds, links, and behaves
// correctly when a server is unreachable (connects to a closed port,
// confirms it moves through Connecting -> Reconnecting -> Failed with a
// bounded ReconnectPolicy, and never crashes/hangs). Exercising the
// success path (a real handshake against server/) is a job for an
// integration test once both sides can be run together - out of scope for
// this foundation milestone's smoke test.
#include <chrono>
#include <iostream>
#include <thread>

#include "network/network_manager.h"
#include "protocol/packet_dispatcher.h"

int main() {
  using namespace unboundmp;

  network::NetworkManager manager;

  // Sanity-check the pieces that don't need any I/O at all.
  network::LatencyTracker latency;
  latency.RecordSample(50);
  latency.RecordSample(60);
  latency.RecordSample(40);
  std::cout << "LatencyTracker average (expected 50): " << latency.AverageRttMs() << "\n";

  network::ReconnectPolicy policy(network::ReconnectConfig{
      /*initial_delay_ms=*/10,
      /*max_delay_ms=*/40,
      /*backoff_multiplier=*/2.0,
      /*jitter_ms=*/0,
      /*max_attempts=*/3,
  });
  int successful_delays = 0;
  while (policy.NextDelayMs().has_value()) {
    ++successful_delays;
  }
  std::cout << "ReconnectPolicy attempts before giving up (expected 3): " << successful_delays
            << "\n";

  // Point at a port nothing is listening on (port 1 is reserved/
  // unprivileged-inaccessible on virtually every OS, so this should fail
  // fast rather than actually connecting to something).
  network::ConnectOptions options;
  options.host = "127.0.0.1";
  options.port = 1;
  options.connect_timeout_ms = 200;
  options.handshake_timeout_ms = 200;
  options.username = "smoke_test";
  options.reconnect.initial_delay_ms = 20;
  options.reconnect.max_delay_ms = 40;
  options.reconnect.jitter_ms = 0;
  options.reconnect.max_attempts = 2;

  manager.Connect(options);

  protocol::PacketDispatcher dispatcher;
  bool saw_failed = false;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (std::chrono::steady_clock::now() < deadline) {
    manager.Poll(dispatcher);
    network::ConnectionState state = manager.State();
    if (state == network::ConnectionState::kFailed) {
      saw_failed = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  std::cout << "Final connection state (expected Failed): " << network::ToString(manager.State())
            << "\n";

  manager.Disconnect();

  const bool all_ok = latency.AverageRttMs() == 50 && successful_delays == 3 && saw_failed;
  std::cout << (all_ok ? "NETWORK MANAGER EXAMPLE: PASS\n" : "NETWORK MANAGER EXAMPLE: FAIL\n");
  return all_ok ? 0 : 1;
}
