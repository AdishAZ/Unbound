#pragma once

#include <asio.hpp>
#include <memory>
#include <thread>
#include "network/multiplayer_client.h"
#include "network/multiplayer_server.h"

namespace unboundmp::network {

class NetworkManager {
 public:
  NetworkManager();
  ~NetworkManager();
  
  NetworkManager(const NetworkManager&) = delete;
  NetworkManager& operator=(const NetworkManager&) = delete;

  void StartServer(uint16_t port);
  void StopServer();

  void ConnectClient(const std::string& host, uint16_t port);
  void DisconnectClient();

  void Shutdown();

  MultiplayerServer* GetServer() const { return server_.get(); }
  MultiplayerClient* GetClient() const { return client_.get(); }
  
  asio::io_context& GetIoContext() { return io_context_; }

 private:
  asio::io_context io_context_;
  std::unique_ptr<asio::io_context::work> work_guard_;
  std::thread worker_thread_;

  std::unique_ptr<MultiplayerServer> server_;
  std::unique_ptr<MultiplayerClient> client_;
};

}  // namespace unboundmp::network
