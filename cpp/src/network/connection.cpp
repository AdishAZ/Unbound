#include "network/connection.h"
#include <iostream>

namespace unboundmp::network {

std::atomic<uint32_t> Connection::next_id_{1};

Connection::Pointer Connection::Create(asio::io_context& io_context, DisconnectCallback on_disconnect) {
  return Pointer(new Connection(io_context, std::move(on_disconnect)));
}

Connection::Connection(asio::io_context& io_context, DisconnectCallback on_disconnect)
    : socket_(io_context), on_disconnect_(std::move(on_disconnect)), id_(next_id_++) {
}

Connection::~Connection() {
  Disconnect();
}

void Connection::Start() {
  connected_.store(true);
  RecordHeartbeat();
  ReadHeader();
}

void Connection::Disconnect() {
  if (connected_.exchange(false)) {
    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    
    if (on_disconnect_) {
      on_disconnect_(shared_from_this());
    }
  }
}

void Connection::SendPacket(const Packet& packet) {
  if (!connected_.load()) return;

  std::vector<uint8_t> encoded = Packet::Encode(packet);
  
  // Custom framing: [4 bytes length][encoded data]
  uint32_t length = static_cast<uint32_t>(encoded.size());
  std::vector<uint8_t> frame(4 + length);
  std::memcpy(frame.data(), &length, 4);
  std::memcpy(frame.data() + 4, encoded.data(), length);

  bytes_out_.fetch_add(frame.size());
  packets_out_.fetch_add(1);

  outgoing_queue_.Push(std::move(frame));

  if (!is_writing_.exchange(true)) {
    WriteAsync();
  }
}

void Connection::WriteAsync() {
  if (!connected_.load()) {
    is_writing_.store(false);
    return;
  }

  auto frame_opt = outgoing_queue_.TryPop();
  if (!frame_opt) {
    is_writing_.store(false);
    return;
  }

  active_write_buffer_ = std::move(*frame_opt);

  auto self(shared_from_this());
  asio::async_write(socket_,
      asio::buffer(active_write_buffer_),
      [this, self](std::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          WriteAsync();
        } else {
          Disconnect();
        }
      });
}

void Connection::ReadHeader() {
  auto self(shared_from_this());
  asio::async_read(socket_,
      asio::buffer(&inbound_size_, sizeof(inbound_size_)),
      [this, self](std::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          if (inbound_size_ > 0 && inbound_size_ < 10 * 1024 * 1024) { // 10MB limit
            inbound_buffer_.resize(inbound_size_);
            ReadBody();
          } else {
            Disconnect();
          }
        } else {
          Disconnect();
        }
      });
}

void Connection::ReadBody() {
  auto self(shared_from_this());
  asio::async_read(socket_,
      asio::buffer(inbound_buffer_),
      [this, self](std::error_code ec, std::size_t /*length*/) {
        if (!ec) {
          try {
            bytes_in_.fetch_add(inbound_buffer_.size() + 4);
            packets_in_.fetch_add(1);
            Packet packet = Packet::Decode(inbound_buffer_);
            incoming_queue_.Push(std::move(packet));
            ReadHeader();
          } catch (...) {
            Disconnect();
          }
        } else {
          Disconnect();
        }
      });
}

void Connection::UpdateLatency(int64_t ping_rtt) {
  latency_.store(ping_rtt);
}

void Connection::RecordHeartbeat() {
  auto now = std::chrono::steady_clock::now().time_since_epoch();
  last_heartbeat_.store(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

}  // namespace unboundmp::network
