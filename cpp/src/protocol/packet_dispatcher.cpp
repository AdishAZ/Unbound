#include "protocol/packet_dispatcher.h"

namespace unboundmp::protocol {

void PacketDispatcher::RegisterHandler(Envelope::PayloadCase payload_case, Handler handler) {
  handlers_[static_cast<int>(payload_case)] = std::move(handler);
}

void PacketDispatcher::RegisterMiddleware(Handler middleware) {
  middleware_.push_back(std::move(middleware));
}

void PacketDispatcher::RegisterUnhandled(Handler handler) {
  unhandled_ = std::move(handler);
}

void PacketDispatcher::Dispatch(const Envelope& envelope) const {
  for (const auto& middleware : middleware_) {
    middleware(envelope);
  }

  const auto it = handlers_.find(static_cast<int>(envelope.payload_case()));
  if (it != handlers_.end()) {
    it->second(envelope);
    return;
  }

  if (unhandled_) {
    unhandled_(envelope);
  }
}

}  // namespace unboundmp::protocol
