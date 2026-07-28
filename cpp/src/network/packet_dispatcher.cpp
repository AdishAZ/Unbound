#include "network/packet_dispatcher.h"

namespace unboundmp::network {

void PacketDispatcher::RegisterHandler(PacketType type, PacketHandler handler) {
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  handlers_[type].push_back(std::move(handler));
}

void PacketDispatcher::Dispatch(Connection::Pointer connection, const Packet& packet) {
  std::vector<PacketHandler> handlers_copy;
  {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    auto it = handlers_.find(packet.type);
    if (it != handlers_.end()) {
      handlers_copy = it->second;
    }
  }

  for (const auto& handler : handlers_copy) {
    if (handler) {
      handler(connection, packet);
    }
  }
}

void PacketDispatcher::ClearHandlers() {
  std::lock_guard<std::mutex> lock(handlers_mutex_);
  handlers_.clear();
}

}  // namespace unboundmp::network
