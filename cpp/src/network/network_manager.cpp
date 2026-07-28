#include "network/network_manager.h"
#include <iostream>

namespace unboundmp::network {

NetworkManager::NetworkManager()
    : work_guard_(std::make_unique<asio::io_context::work>(io_context_)) {
  worker_thread_ = std::thread([this]() {
    io_context_.run();
  });
}

NetworkManager::~NetworkManager() {
  Shutdown();
}

void NetworkManager::StartServer(uint16_t port) {
  if (!server_) {
    server_ = std::make_unique<MultiplayerServer>(io_context_);
  }
  server_->Start(port);
}

void NetworkManager::StopServer() {
  if (server_) {
    server_->Stop();
    server_.reset();
  }
}

void NetworkManager::ConnectClient(const std::string& host, uint16_t port) {
  if (!client_) {
    client_ = std::make_unique<MultiplayerClient>(io_context_);
  }
  client_->Connect(host, port);
}

void NetworkManager::DisconnectClient() {
  if (client_) {
    client_->Disconnect();
    client_.reset();
  }
}

void NetworkManager::Shutdown() {
  StopServer();
  DisconnectClient();
  
  if (work_guard_) {
    work_guard_.reset();
    io_context_.stop();
    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }
  }
}

}  // namespace unboundmp::network
