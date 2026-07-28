#include "network/packet.h"
#include "network/packet_dispatcher.h"
#include "network/multiplayer_client.h"
#include "network/multiplayer_server.h"
#include "client/network/client_state_manager.h"
#include "server/network/player_registry.h"
#include "server/utils/logger.h"
#include "server/config/server_config.h"
#include "server/database/database.h"
#include "server/accounts/account_manager.h"
#include "server/characters/character_manager.h"
#include "server/sessions/session_manager.h"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <cassert>
#include <atomic>

using namespace unboundmp::network;
using namespace unboundmp::server;
using namespace unboundmp::client;

void Verify(bool condition, const std::string& message) {
  if (condition) {
    Logger::Info("[PASS] " + message);
  } else {
    Logger::Error("[FAIL] " + message);
    std::exit(1);
  }
}

int main() {
  Logger::Info("Starting Stage 6 Multiplayer Sync Test...");

  ServerConfig config;
  std::shared_ptr<DatabasePool> db_pool;
  try {
    db_pool = std::make_shared<DatabasePool>(config.GetConnectionString(), 2);
    db_pool->InitializeSchema();
  } catch (...) {
    Logger::Warn("Database not available. Skipping Stage 6 automated test because it relies on the DB for auth.");
    return 0; // Skip test safely in environments without Postgres
  }

  asio::io_context server_io;
  MultiplayerServer server(server_io);
  
  auto account_manager = std::make_shared<AccountManager>(db_pool);
  auto character_manager = std::make_shared<CharacterManager>(db_pool);
  auto session_manager = std::make_shared<SessionManager>();
  auto player_registry = std::make_shared<PlayerRegistry>();
  PacketDispatcher server_dispatcher;
  
  // Register Server Handlers
  server_dispatcher.RegisterHandler(PacketType::kAuthRequest, [&](Connection::Pointer conn, const Packet& p) {
    AuthRequestPacket req = AuthRequestPacket::Deserialize(p.payload);
    auto acc = account_manager->Login(req.username, req.password);
    AuthResponsePacket resp;
    if (acc) {
      std::string token = session_manager->CreateSession(*acc, conn);
      resp.success = true;
      resp.token = token;
      resp.account_id = acc->id;
      
      OnlinePlayer op;
      op.session_token = token;
      op.connection_id = conn->GetId();
      op.account_id = acc->id;
      op.current_connection_state = PresenceState::Online;
      player_registry->AddPlayer(op);
      
      // Broadcast presence
      PlayerPresencePacket pres;
      pres.account_id = acc->id;
      pres.state = PresenceState::Online;
      Packet pres_pkt;
      pres_pkt.type = PacketType::kPlayerPresence;
      pres_pkt.payload = pres.Serialize();
      for (const auto& other : player_registry->GetAllPlayers()) {
        if (other.account_id != acc->id) {
          auto other_session = session_manager->GetSessionByToken(other.session_token);
          if (other_session && other_session->connection) {
            other_session->connection->SendPacket(pres_pkt);
          }
        }
      }
    } else {
      resp.success = false;
    }
    Packet resp_pkt;
    resp_pkt.type = PacketType::kAuthResponse;
    resp_pkt.payload = resp.Serialize();
    conn->SendPacket(resp_pkt);
  });

  server_dispatcher.RegisterHandler(PacketType::kHeartbeat, [&](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (session) {
      player_registry->UpdateHeartbeat(session->account.id, 12345); // Dummy time
    }
  });

  server.Start(12346);
  std::thread server_thread([&server_io]() {
    try { server_io.run(); } catch (...) {}
  });

  // Client 1 setup
  asio::io_context client1_io;
  MultiplayerClient client1(client1_io);
  ClientStateManager state1;
  PacketDispatcher dispatcher1;
  std::atomic<bool> client1_auth_success{false};

  dispatcher1.RegisterHandler(PacketType::kAuthResponse, [&](Connection::Pointer conn, const Packet& p) {
    AuthResponsePacket resp = AuthResponsePacket::Deserialize(p.payload);
    if (resp.success) {
      state1.SetState(ClientState::kLoggedIn);
      state1.SetSessionToken(resp.token);
      client1_auth_success = true;
    }
  });

  std::thread client1_thread([&client1_io]() {
    try { client1_io.run(); } catch (...) {}
  });

  // Create test user
  std::string user1 = "test_user1";
  std::string pass1 = "pass123";
  account_manager->CreateAccount(user1, pass1);

  // Connect Client 1
  client1.Connect("127.0.0.1", 12346);
  state1.SetState(ClientState::kConnecting);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  if (client1.IsConnected()) {
    state1.SetState(ClientState::kConnected);
    
    AuthRequestPacket req;
    req.username = user1;
    req.password = pass1;
    Packet p;
    p.type = PacketType::kAuthRequest;
    p.payload = req.Serialize();
    client1.SendPacket(p);
  }

  // Poll for a bit
  for (int i = 0; i < 10; ++i) {
    server.PollIncoming(&server_dispatcher);
    while (auto pkt = client1.ReceivePacket()) {
      dispatcher1.Dispatch(nullptr, *pkt);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  Verify(client1_auth_success, "Client 1 authenticated successfully");
  Verify(state1.GetState() == ClientState::kLoggedIn, "Client 1 state is LoggedIn");
  Verify(player_registry->GetAllPlayers().size() == 1, "PlayerRegistry has 1 player");

  // Send Heartbeat using session token
  Packet hb;
  hb.type = PacketType::kHeartbeat;
  hb.session_token = state1.GetSessionToken();
  client1.SendPacket(hb);

  for (int i = 0; i < 5; ++i) {
    server.PollIncoming(&server_dispatcher);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  auto op = player_registry->GetPlayerByToken(state1.GetSessionToken());
  Verify(op.has_value() && op->last_heartbeat == 12345, "Heartbeat updated PlayerRegistry");

  // Cleanup
  Logger::Info("Graceful shutdown...");
  client1.Disconnect();
  server.Stop();
  
  client1_io.stop();
  server_io.stop();
  
  if (client1_thread.joinable()) client1_thread.join();
  if (server_thread.joinable()) server_thread.join();
  
  account_manager->DeleteAccount(op->account_id);
  
  Logger::Info("Test finished successfully.");
  return 0;
}
