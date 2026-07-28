#include "network/multiplayer_server.h"
#include "network/packet_dispatcher.h"
#include <iostream>

namespace unboundmp::network {

MultiplayerServer::MultiplayerServer(asio::io_context& io_context)
    : io_context_(io_context) {}

MultiplayerServer::~MultiplayerServer() {
  Stop();
}

void MultiplayerServer::Start(uint16_t port) {
  if (is_running_.exchange(true)) return;
  
  asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
  acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(io_context_, endpoint);
  
  StartAccept();
}

void MultiplayerServer::Stop() {
  if (!is_running_.exchange(false)) return;
  
  if (acceptor_) {
    asio::error_code ec;
    acceptor_->close(ec);
    acceptor_.reset();
  }
  
  std::unordered_map<uint32_t, Connection::Pointer> connections_to_close;
  {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_to_close = std::move(connections_);
    connections_.clear();
  }
  
  for (auto& [id, conn] : connections_to_close) {
    conn->Disconnect();
  }
}

void MultiplayerServer::StartAccept() {
  auto new_connection = Connection::Create(io_context_, [this](Connection::Pointer conn) {
    RemoveConnection(conn);
  });
  
  acceptor_->async_accept(new_connection->Socket(),
      [this, new_connection](const std::error_code& error) {
        HandleAccept(new_connection, error);
      });
}

void MultiplayerServer::HandleAccept(Connection::Pointer new_connection, const std::error_code& error) {
  if (!is_running_.load()) return;
  
  if (!error) {
    {
      std::lock_guard<std::mutex> lock(connections_mutex_);
      connections_[new_connection->GetId()] = new_connection;
    }
    new_connection->Start();
  }
  
  StartAccept();
}

void MultiplayerServer::RemoveConnection(Connection::Pointer connection) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  connections_.erase(connection->GetId());
}

void MultiplayerServer::BroadcastPacket(const Packet& packet) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto& [id, conn] : connections_) {
    conn->SendPacket(packet);
  }
}


void MultiplayerServer::PollIncoming(PacketDispatcher* dispatcher) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto& [id, conn] : connections_) {
    while (auto pkt_opt = conn->IncomingQueue().TryPop()) {
      if (dispatcher) {
        dispatcher->Dispatch(conn, *pkt_opt);
      }
      if (pkt_opt->type == PacketType::kPing) {
        // Send pong
        Packet pong;
        pong.type = PacketType::kPong;
        pong.sequence_number = pkt_opt->sequence_number;
        conn->SendPacket(pong);
      }
    }
  }
}

}  // namespace unboundmp::network
