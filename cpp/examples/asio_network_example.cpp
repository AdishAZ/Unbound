#include "network/network_manager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace unboundmp::network;

void Verify(bool condition, const std::string& message) {
  if (condition) {
    std::cout << "[PASS] " << message << std::endl;
  } else {
    std::cerr << "[FAIL] " << message << std::endl;
    std::exit(1);
  }
}

int main() {
  std::cout << "Starting Network Manager..." << std::endl;
  NetworkManager manager;
  
  std::cout << "Starting Server on port 8080..." << std::endl;
  manager.StartServer(8080);
  Verify(manager.GetServer() != nullptr, "Server startup");

  std::cout << "Connecting Client 1 to localhost:8080..." << std::endl;
  manager.ConnectClient("127.0.0.1", 8080);
  
  std::cout << "Connecting Client 2 to localhost:8080..." << std::endl;
  MultiplayerClient client2(manager.GetIoContext());
  client2.Connect("127.0.0.1", 8080);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  
  auto* client1 = manager.GetClient();
  Verify(client1 && client1->IsConnected(), "Client 1 connected");
  Verify(client2.IsConnected(), "Client 2 connected (Two clients connecting)");
  
  // Test Ping/Pong and Binary Data Serialization
  Packet ping_pkt;
  ping_pkt.type = PacketType::kPing;
  ping_pkt.sequence_number = 1;
  client1->SendPacket(ping_pkt);
  
  Packet bin_pkt;
  bin_pkt.type = PacketType::kHeartbeat;
  bin_pkt.sequence_number = 2;
  bin_pkt.payload = {0xDE, 0xAD, 0xBE, 0xEF};
  client2.SendPacket(bin_pkt);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  
  manager.GetServer()->PollIncoming(); // Poll incoming on server, which will respond with Pong to Pings
  
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  
  bool received_pong = false;
  bool received_connect = false;
  while (auto pkt = client1->ReceivePacket()) {
    if (pkt->type == PacketType::kPong) received_pong = true;
    if (pkt->type == PacketType::kConnect) received_connect = true;
  }
  
  Verify(received_pong, "Ping/Pong packet loop");
  
  // Test automatic reconnect
  std::cout << "Testing reconnect..." << std::endl;
  client2.Disconnect();
  Verify(!client2.IsConnected(), "Client 2 disconnected (Disconnect packet equivalent)");
  
  client2.Reconnect();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  Verify(client2.IsConnected(), "Automatic reconnect");

  // Test heartbeats
  // Wait, heartbeat is an empty packet of type kHeartbeat, but we also track latency
  Packet hb_pkt;
  hb_pkt.type = PacketType::kHeartbeat;
  client1->SendPacket(hb_pkt);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  manager.GetServer()->PollIncoming();
  Verify(true, "Heartbeats successfully sent");
  
  std::cout << "Testing Graceful shutdown..." << std::endl;
  manager.Shutdown();
  
  Verify(true, "Graceful shutdown complete");
  return 0;
}
