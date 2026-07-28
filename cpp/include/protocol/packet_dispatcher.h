#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "packets.pb.h"

namespace unboundmp::protocol {

// Dispatches decoded Envelopes to handlers registered per payload type
// (Envelope::PayloadCase). This is the single place that maps "what kind
// of packet is this" to "what code handles it" — both the client and the
// server register their handlers against the same dispatcher type, they
// just register different callbacks.
//
// Not thread-safe. If packets arrive on a network thread and need to be
// handled on a different thread (e.g. the main/render thread on the
// client), the caller is responsible for hopping threads before calling
// Dispatch() — this class does not do any queuing itself.
class PacketDispatcher {
 public:
  using Handler = std::function<void(const Envelope&)>;

  // Registers `handler` to be invoked for every Envelope whose payload_case
  // matches `payload_case`. Registering a second handler for the same case
  // replaces the first (last registration wins) — this is intentional so
  // tests/tools can override a single handler without rebuilding the whole
  // dispatcher.
  void RegisterHandler(Envelope::PayloadCase payload_case, Handler handler);

  // Registers a handler invoked for every Envelope regardless of payload
  // type, *before* the type-specific handler runs. Useful for cross-cutting
  // concerns: logging, metrics, sequence-number bookkeeping. Multiple
  // middleware handlers may be registered; they run in registration order.
  void RegisterMiddleware(Handler middleware);

  // Routes `envelope` to whichever handler was registered for its
  // payload_case(). If no handler is registered for that case, invokes
  // the fallback handler set via RegisterUnhandled(), if any; otherwise
  // the envelope is silently dropped (this is a deliberate default so an
  // unregistered packet type never crashes either side of the connection).
  void Dispatch(const Envelope& envelope) const;

  // Registers a handler invoked when Dispatch() is called with a
  // payload_case that has no registered handler. Useful for logging
  // unexpected packets during development.
  void RegisterUnhandled(Handler handler);

 private:
  std::unordered_map<int, Handler> handlers_;
  std::vector<Handler> middleware_;
  Handler unhandled_;
};

}  // namespace unboundmp::protocol
