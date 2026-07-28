#pragma once

#include <asio.hpp>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include "network/connection.h"

namespace unboundmp::network {

class PacketDispatcher;

class MultiplayerServer {
 public:
  explicit MultiplayerServer(asio::io_context& io_context);
  ~MultiplayerServer();

  void Start(uint16_t port);
  void Stop();
  void BroadcastPacket(const Packet& packet);
  
  void PollIncoming(PacketDispatcher* dispatcher = nullptr);

 private:
  void StartAccept();
  void HandleAccept(Connection::Pointer new_connection, const std::error_code& error);
  void RemoveConnection(Connection::Pointer connection);

  asio::io_context& io_context_;
  std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
  
  std::mutex connections_mutex_;
  std::unordered_map<uint32_t, Connection::Pointer> connections_;
  
  std::atomic<bool> is_running_{false};
};

}  // namespace unboundmp::network
