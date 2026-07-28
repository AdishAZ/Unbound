#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <atomic>
#include "network/connection.h"

namespace unboundmp::network {

class MultiplayerClient {
 public:
  explicit MultiplayerClient(asio::io_context& io_context);
  ~MultiplayerClient();

  void Connect(const std::string& host, uint16_t port);
  void Disconnect();
  void Reconnect();
  
  bool IsConnected() const;
  int64_t GetPing() const;
  
  void SendPacket(const Packet& packet);
  std::optional<Packet> ReceivePacket();
  
  void UpdateStats(float dt);
  float GetBandwidthInKBps() const { return in_kbps_; }
  float GetBandwidthOutKBps() const { return out_kbps_; }
  int GetPacketRateIn() const { return in_pps_; }
  int GetPacketRateOut() const { return out_pps_; }
  uint32_t GetQueueSize() const;

 private:
  void HandleResolve(const std::error_code& error, asio::ip::tcp::resolver::results_type results);
  void HandleConnect(const std::error_code& error, const asio::ip::tcp::endpoint& endpoint);
  void OnDisconnect(Connection::Pointer conn);

  asio::io_context& io_context_;
  asio::ip::tcp::resolver resolver_;
  
  Connection::Pointer connection_;
  
  std::string last_host_;
  uint16_t last_port_ = 0;
  
  std::atomic<bool> is_connecting_{false};
  
  float stats_timer_ = 0.0f;
  uint64_t last_bytes_in_ = 0;
  uint64_t last_bytes_out_ = 0;
  uint64_t last_packets_in_ = 0;
  uint64_t last_packets_out_ = 0;
  
  float in_kbps_ = 0.0f;
  float out_kbps_ = 0.0f;
  int in_pps_ = 0;
  int out_pps_ = 0;
};

}  // namespace unboundmp::network
