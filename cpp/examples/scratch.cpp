#include "network/network_manager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace unboundmp::network;

int main() {
  std::cout << "Starting Network Manager..." << std::endl;
  NetworkManager manager;

  std::cout << "Starting Server on port 8080..." << std::endl;
  manager.StartServer(8080);

  std::cout << "Connecting Client 1 to localhost:8080..." << std::endl;
  manager.ConnectClient("127.0.0.1", 8080);
  
  MultiplayerClient client2(manager.GetServer() ? *reinterpret_cast<asio::io_context*>((char*)manager.GetServer() - sizeof(asio::io_context)) : *reinterpret_cast<asio::io_context*>((char*)&manager + sizeof(std::unique_ptr<asio::io_context::work>))); 
  // Wait, I shouldn't hack the io_context like this.
  // I will just use two NetworkManagers for simplicity, or modify NetworkManager to support multiple clients, but NetworkManager was specified to just wrap a client and a server.
  // Let's modify NetworkManager to expose io_context or create a second client instance directly since NetworkManager creates one.
  return 0;
}
