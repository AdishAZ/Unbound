#pragma once

#include <asio.hpp>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>
#include "network/packet.h"
#include "network/thread_safe_queue.h"

namespace unboundmp::network {

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  using Pointer = std::shared_ptr<Connection>;
  using DisconnectCallback = std::function<void(Pointer)>;

  static Pointer Create(asio::io_context& io_context, DisconnectCallback on_disconnect);

  ~Connection();

  asio::ip::tcp::socket& Socket() { return socket_; }

  void Start();
  void Disconnect();
  void SendPacket(const Packet& packet);
  
  ThreadSafeQueue<Packet>& IncomingQueue() { return incoming_queue_; }

  bool IsConnected() const { return connected_.load(); }
  uint32_t GetId() const { return id_; }
  
  // Latency & Heartbeat
  void UpdateLatency(int64_t ping_rtt);
  int64_t GetLatency() const { return latency_.load(); }
  int64_t GetLastHeartbeat() const { return last_heartbeat_.load(); }
  void RecordHeartbeat();
  
  // Stats getters
  uint64_t GetBytesIn() const { return bytes_in_.load(); }
  uint64_t GetBytesOut() const { return bytes_out_.load(); }
  uint64_t GetPacketsIn() const { return packets_in_.load(); }
  uint64_t GetPacketsOut() const { return packets_out_.load(); }
  uint32_t GetQueueSize() const { return outgoing_queue_.Size(); }

 private:
  Connection(asio::io_context& io_context, DisconnectCallback on_disconnect);

  void ReadHeader();
  void ReadBody();
  void WriteAsync();

  asio::ip::tcp::socket socket_;
  DisconnectCallback on_disconnect_;
  
  std::atomic<bool> connected_{false};
  uint32_t id_;

  // Buffer for length-prefix framing. 
  // Custom packet binary format:
  // [4 bytes size N][N bytes encoded Packet]
  uint32_t inbound_size_ = 0;
  std::vector<uint8_t> inbound_buffer_;

  ThreadSafeQueue<Packet> incoming_queue_;
  ThreadSafeQueue<std::vector<uint8_t>> outgoing_queue_;
  
  std::atomic<bool> is_writing_{false};
  std::vector<uint8_t> active_write_buffer_;

  std::atomic<int64_t> latency_{0};
  std::atomic<int64_t> last_heartbeat_{0};

  std::atomic<uint64_t> bytes_in_{0};
  std::atomic<uint64_t> bytes_out_{0};
  std::atomic<uint64_t> packets_in_{0};
  std::atomic<uint64_t> packets_out_{0};

  static std::atomic<uint32_t> next_id_;
};

}  // namespace unboundmp::network
