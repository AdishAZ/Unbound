#include "network/multiplayer_client.h"
#include <iostream>

namespace unboundmp::network {

MultiplayerClient::MultiplayerClient(asio::io_context& io_context)
    : io_context_(io_context), resolver_(io_context) {}

MultiplayerClient::~MultiplayerClient() {
  Disconnect();
}

void MultiplayerClient::Connect(const std::string& host, uint16_t port) {
  if (is_connecting_.exchange(true) || IsConnected()) return;
  
  last_host_ = host;
  last_port_ = port;
  
  connection_ = Connection::Create(io_context_, [this](Connection::Pointer conn) {
    OnDisconnect(conn);
  });
  
  resolver_.async_resolve(host, std::to_string(port),
      [this](const std::error_code& error, asio::ip::tcp::resolver::results_type results) {
        HandleResolve(error, results);
      });
}

void MultiplayerClient::Disconnect() {
  if (connection_) {
    connection_->Disconnect();
  }
}

void MultiplayerClient::Reconnect() {
  if (!last_host_.empty() && last_port_ != 0) {
    Connect(last_host_, last_port_);
  }
}

bool MultiplayerClient::IsConnected() const {
  return connection_ && connection_->IsConnected();
}

int64_t MultiplayerClient::GetPing() const {
  if (connection_) {
    return connection_->GetLatency();
  }
  return 0;
}

void MultiplayerClient::SendPacket(const Packet& packet) {
  if (connection_) {
    connection_->SendPacket(packet);
  }
}

std::optional<Packet> MultiplayerClient::ReceivePacket() {
  if (connection_) {
    return connection_->IncomingQueue().TryPop();
  }
  return std::nullopt;
}

uint32_t MultiplayerClient::GetQueueSize() const {
  if (connection_) return connection_->GetQueueSize();
  return 0;
}

void MultiplayerClient::UpdateStats(float dt) {
  stats_timer_ += dt;
  if (stats_timer_ >= 1.0f) {
    if (connection_) {
      uint64_t current_bytes_in = connection_->GetBytesIn();
      uint64_t current_bytes_out = connection_->GetBytesOut();
      uint64_t current_packets_in = connection_->GetPacketsIn();
      uint64_t current_packets_out = connection_->GetPacketsOut();
      
      in_kbps_ = ((current_bytes_in - last_bytes_in_) / 1024.0f) / stats_timer_;
      out_kbps_ = ((current_bytes_out - last_bytes_out_) / 1024.0f) / stats_timer_;
      in_pps_ = static_cast<int>((current_packets_in - last_packets_in_) / stats_timer_);
      out_pps_ = static_cast<int>((current_packets_out - last_packets_out_) / stats_timer_);
      
      last_bytes_in_ = current_bytes_in;
      last_bytes_out_ = current_bytes_out;
      last_packets_in_ = current_packets_in;
      last_packets_out_ = current_packets_out;
    } else {
      in_kbps_ = 0.0f;
      out_kbps_ = 0.0f;
      in_pps_ = 0;
      out_pps_ = 0;
    }
    stats_timer_ = 0.0f;
  }
}

void MultiplayerClient::HandleResolve(const std::error_code& error, asio::ip::tcp::resolver::results_type results) {
  if (!error) {
    asio::async_connect(connection_->Socket(), results,
        [this](const std::error_code& error, const asio::ip::tcp::endpoint& endpoint) {
          HandleConnect(error, endpoint);
        });
  } else {
    is_connecting_.store(false);
  }
}

void MultiplayerClient::HandleConnect(const std::error_code& error, const asio::ip::tcp::endpoint& /*endpoint*/) {
  is_connecting_.store(false);
  if (!error) {
    connection_->Start();
    Packet connect_pkt;
    connect_pkt.type = PacketType::kConnect;
    SendPacket(connect_pkt);
  }
}

void MultiplayerClient::OnDisconnect(Connection::Pointer /*conn*/) {
  // Reconnect logic could be added here
}

}  // namespace unboundmp::network
