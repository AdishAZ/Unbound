#pragma once

#include "network/packet.h"
#include "network/connection.h"
#include <functional>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace unboundmp::network {

using PacketHandler = std::function<void(Connection::Pointer, const Packet&)>;

class PacketDispatcher {
 public:
  PacketDispatcher() = default;

  void RegisterHandler(PacketType type, PacketHandler handler);
  void Dispatch(Connection::Pointer connection, const Packet& packet);
  
  void ClearHandlers();

 private:
  std::mutex handlers_mutex_;
  std::unordered_map<PacketType, std::vector<PacketHandler>> handlers_;
};

}  // namespace unboundmp::network
