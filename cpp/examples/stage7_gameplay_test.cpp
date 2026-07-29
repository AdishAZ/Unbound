#include "gameplay/gameplay_sync_manager.h"
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
#include "emulator/emulator_core.h"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

using namespace unboundmp::network;
using namespace unboundmp::server;
using namespace unboundmp::client;
using namespace unboundmp::gameplay;
using namespace unboundmp::emulator;

void Verify(bool condition, const std::string& message) {
  if (condition) {
    Logger::Info("[PASS] " + message);
  } else {
    Logger::Error("[FAIL] " + message);
    std::exit(1);
  }
}

// Dummy core that mocks memory reads for Position, Map, Direction
class MockCore : public IEmulatorCore {
public:
    uint32_t map_id = 0;
    float x = 0.0f;
    float y = 0.0f;
    uint8_t direction = 0;
    uint8_t move_state = 0;

    // Address constants from memory_api.h/address_table.h used by unboundmp_memory
    // The readers in unboundmp_memory rely on these specific addresses
    // We will just return successful data for the exact addresses requested.
    
    uint8_t ReadU8(uint32_t address) override { 
        // Mock specific addresses if known, else 0
        return 0; 
    }
    uint16_t ReadU16(uint32_t address) override { return 0; }
    uint32_t ReadU32(uint32_t address) override { return 0; }
    void ReadBytes(uint32_t address, uint8_t* out, size_t length) override {}
    
    void WriteU8(uint32_t address, uint8_t value) override {}
    void WriteU16(uint32_t address, uint16_t value) override {}
    void WriteU32(uint32_t address, uint32_t value) override {}
    void WriteBytes(uint32_t address, const uint8_t* data, size_t length) override {}
    
    // Stubs
    const void* GetVideoBuffer() const override { return nullptr; }
    size_t GetVideoPitch() const override { return 0; }
    EmulatorResult Initialize() override { return EmulatorResult::Success(); }
    EmulatorResult LoadRom(const std::string& path) override { return EmulatorResult::Success(); }
    EmulatorResult LoadSave(const std::string& path) override { return EmulatorResult::Success(); }
    EmulatorResult Start() override { return EmulatorResult::Success(); }
    EmulatorResult Pause() override { return EmulatorResult::Success(); }
    EmulatorResult Resume() override { return EmulatorResult::Success(); }
    EmulatorResult Reset() override { return EmulatorResult::Success(); }
    EmulatorResult Stop() override { return EmulatorResult::Success(); }
    void Shutdown() override {}
    EmulatorState State() const override { return EmulatorState::kRunning; }
    void SetInput(const InputState& state) override {}
    std::optional<std::string> GameTitle() const override { return "Pokemon Unbound"; }
    std::optional<std::string> GameCode() const override { return "BPRE"; }
    bool IsRunning() const override { return true; }
    bool IsPaused() const override { return false; }
    bool IsLoaded() const override { return true; }
    uint32_t GetFrameCount() const override { return 0; }
    float GetFPS() const override { return 60.0f; }
    float GetCurrentSpeed() const override { return 1.0f; }
    uint32_t RomCrc32() const override { return 0; }
    size_t RomSize() const override { return 0; }
    std::optional<std::string> SaveType() const override { return ""; }
    bool HasSaveState(int slot) const override { return false; }
    EmulatorResult DeleteSaveState(int slot) override { return EmulatorResult::Success(); }
    std::vector<int> ListSaveStates() const override { return {}; }
    std::optional<SaveStateMetadata> GetSaveStateMetadata(int slot) const override { return std::nullopt; }
    size_t RegisterEventCallback(EventCallback cb) override { return 0; }
    void UnregisterEventCallback(size_t id) override {}
};


int main() {
  Logger::Info("Starting Stage 7 Gameplay Sync Test...");

  asio::io_context server_io;
  MultiplayerServer server(server_io);
  
  auto session_manager = std::make_shared<SessionManager>();
  auto player_registry = std::make_shared<PlayerRegistry>();
  PacketDispatcher server_dispatcher;
  
  // Set up Gameplay Sync Manager
  GameplaySyncManager server_gameplay(player_registry, session_manager);
  server_gameplay.RegisterServerHandlers(server_dispatcher);

  // Mock auth handler directly putting player in registry without DB
  server_dispatcher.RegisterHandler(PacketType::kAuthRequest, [&](Connection::Pointer conn, const Packet& p) {
    AuthRequestPacket req = AuthRequestPacket::Deserialize(p.payload);
    std::string token = "token_" + req.username;
    
    AuthResponsePacket resp;
    resp.success = true;
    resp.token = token;
    
    // Hash username for a fake account_id
    uint64_t fake_id = std::hash<std::string>{}(req.username);
    resp.account_id = fake_id;
    
    OnlinePlayer op;
    op.connection_id = conn->GetId();
    op.account_id = fake_id;
    op.current_connection_state = PresenceState::Online;
    
    // Also create a fake session to satisfy GameplaySyncManager
    Account fake_account;
    fake_account.id = fake_id;
    fake_account.username = req.username;
    
    // We already generated a token, but SessionManager::CreateSession generates its own.
    // So we just call CreateSession and use the returned token.
    token = session_manager->CreateSession(fake_account, conn);
    resp.token = token;
    
    // Fix up the OnlinePlayer with the real token
    op.session_token = token;
    player_registry->AddPlayer(op);
    
    Packet resp_pkt;
    resp_pkt.type = PacketType::kAuthResponse;
    resp_pkt.payload = resp.Serialize();
    conn->SendPacket(resp_pkt);
  });

  server.Start(12347);
  std::thread server_thread([&server_io]() {
    try { server_io.run(); } catch (...) {}
  });

  // Client 1
  asio::io_context client1_io;
  MultiplayerClient client1(client1_io);
  ClientStateManager state1;
  PacketDispatcher dispatcher1;
  std::shared_ptr<MockCore> core1 = std::make_shared<MockCore>();
  auto remote_player_manager1 = std::make_shared<RemotePlayerManager>();
  GameplaySyncManager client_gameplay1(core1, remote_player_manager1);
  client_gameplay1.RegisterClientHandlers(dispatcher1);

  dispatcher1.RegisterHandler(PacketType::kAuthResponse, [&](Connection::Pointer conn, const Packet& p) {
    AuthResponsePacket resp = AuthResponsePacket::Deserialize(p.payload);
    if (resp.success) {
      state1.SetState(ClientState::kLoggedIn);
      state1.SetSessionToken(resp.token);
    }
  });

  std::thread client1_thread([&client1_io]() {
    try { client1_io.run(); } catch (...) {}
  });

  // Client 2
  asio::io_context client2_io;
  MultiplayerClient client2(client2_io);
  ClientStateManager state2;
  PacketDispatcher dispatcher2;
  std::shared_ptr<MockCore> core2 = std::make_shared<MockCore>();
  auto remote_player_manager2 = std::make_shared<RemotePlayerManager>();
  GameplaySyncManager client_gameplay2(core2, remote_player_manager2);
  client_gameplay2.RegisterClientHandlers(dispatcher2);

  dispatcher2.RegisterHandler(PacketType::kAuthResponse, [&](Connection::Pointer conn, const Packet& p) {
    AuthResponsePacket resp = AuthResponsePacket::Deserialize(p.payload);
    if (resp.success) {
      state2.SetState(ClientState::kLoggedIn);
      state2.SetSessionToken(resp.token);
    }
  });

  std::thread client2_thread([&client2_io]() {
    try { client2_io.run(); } catch (...) {}
  });

  std::string user1 = "stage7_u1"; std::string pass1 = "p1";
  std::string user2 = "stage7_u2"; std::string pass2 = "p2";

  client1.Connect("127.0.0.1", 12347);
  client2.Connect("127.0.0.1", 12347);
  
  while (!client1.IsConnected() || !client2.IsConnected()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } // wait longer for connection

  if (client1.IsConnected()) {
    AuthRequestPacket req; req.username = user1; req.password = pass1;
    Packet p; p.type = PacketType::kAuthRequest; p.payload = req.Serialize();
    client1.SendPacket(p);
  }
  if (client2.IsConnected()) {
    AuthRequestPacket req; req.username = user2; req.password = pass2;
    Packet p; p.type = PacketType::kAuthRequest; p.payload = req.Serialize();
    client2.SendPacket(p);
  }

  for (int i = 0; i < 10; ++i) {
    server.PollIncoming(&server_dispatcher);
    while (auto pkt = client1.ReceivePacket()) dispatcher1.Dispatch(nullptr, *pkt);
    while (auto pkt = client2.ReceivePacket()) dispatcher2.Dispatch(nullptr, *pkt);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  Verify(state1.GetState() == ClientState::kLoggedIn, "Client 1 LoggedIn");
  Verify(state2.GetState() == ClientState::kLoggedIn, "Client 2 LoggedIn");

  // Client 1 transitions to map 5
  MapTransitionPacket mt1;
  mt1.map_id = 5; mt1.x = 10; mt1.y = 10;
  Packet p_mt1; p_mt1.type = PacketType::kMapTransition; p_mt1.session_token = state1.GetSessionToken(); p_mt1.payload = mt1.Serialize();
  client1.SendPacket(p_mt1);

  // Client 2 transitions to map 5
  MapTransitionPacket mt2;
  mt2.map_id = 5; mt2.x = 12; mt2.y = 10;
  Packet p_mt2; p_mt2.type = PacketType::kMapTransition; p_mt2.session_token = state2.GetSessionToken(); p_mt2.payload = mt2.Serialize();
  client2.SendPacket(p_mt2);

  for (int i = 0; i < 10; ++i) {
    server.PollIncoming(&server_dispatcher);
    while (auto pkt = client1.ReceivePacket()) dispatcher1.Dispatch(nullptr, *pkt);
    while (auto pkt = client2.ReceivePacket()) dispatcher2.Dispatch(nullptr, *pkt);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Client 1 should see Client 2
  auto c1_players = client_gameplay1.GetRemotePlayerManager()->GetPlayersOnMap(5);
  Verify(c1_players.size() == 1, "Client 1 sees Client 2 on Map 5");

  // Client 2 moves
  PlayerStatePacket mv2;
  mv2.x = 15; mv2.y = 10; mv2.direction = 1; mv2.movement_state = 1;
  Packet p_mv2; p_mv2.type = PacketType::kPlayerState; p_mv2.session_token = state2.GetSessionToken(); p_mv2.payload = mv2.Serialize();
  client2.SendPacket(p_mv2);

  for (int i = 0; i < 5; ++i) {
    server.PollIncoming(&server_dispatcher);
    while (auto pkt = client1.ReceivePacket()) dispatcher1.Dispatch(nullptr, *pkt);
    while (auto pkt = client2.ReceivePacket()) dispatcher2.Dispatch(nullptr, *pkt);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  // Client 1 checks remote manager
  c1_players = client_gameplay1.GetRemotePlayerManager()->GetPlayersOnMap(5);
  Verify(c1_players[0].target_x == 15, "Client 1 received Client 2's movement");

  // Cleanup
  Logger::Info("Graceful shutdown...");
  client1.Disconnect();
  client2.Disconnect();
  server.Stop();
  
  client1_io.stop();
  client2_io.stop();
  server_io.stop();
  
  if (client1_thread.joinable()) client1_thread.join();
  if (client2_thread.joinable()) client2_thread.join();
  if (server_thread.joinable()) server_thread.join();
  
  Logger::Info("Test finished successfully.");
  return 0;
}
