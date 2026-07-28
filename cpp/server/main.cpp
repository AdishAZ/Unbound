#include "config/server_config.h"
#include "database/database.h"
#include "authentication/authentication.h"
#include "accounts/account_manager.h"
#include "database/character_repository.h"
#include "sessions/session_manager.h"
#include "network/network_manager.h"
#include "network/packet_dispatcher.h"
#include "network/player_registry.h"
#include "persistence/player_persistence.h"
#include "persistence/autosave_manager.h"
#include "world/world_server.h"
#include "utils/logger.h"
#include "server_context.h"
#include "gameplay/inventory_manager.h"
#include "gameplay/item_manager.h"

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace unboundmp;
using namespace unboundmp::server;
using namespace unboundmp::network;
using namespace unboundmp::persistence;

int main() {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
  Logger::Info("Starting UnboundMP Server...");

  ServerConfig config = ServerConfig::Load("server.conf");
  
  unboundmp::server::ServerContext ctx;
  try {
      ctx.Initialize(config.GetConnectionString());
  } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR: Server initialization failed: " << e.what() << std::endl;
      Logger::Error(std::string("Server initialization failed: ") + e.what());
      return 1;
  }
  
  auto account_manager = ctx.GetAccountManager();
  auto character_repo = ctx.GetCharacterRepository();
  auto session_manager = ctx.GetSessionManager();
  auto player_registry = ctx.GetPlayerRegistry();
  auto dispatcher = ctx.GetPacketDispatcher();
  auto world_server = ctx.GetWorldServer();
  auto db_pool = ctx.GetDatabasePool();
  auto inventory_mgr = ctx.GetInventoryManager();
  auto item_mgr = ctx.GetItemManager();
  
  uint64_t current_server_tick = 0;

  auto SendWorldSnapshot = [&](uint64_t account_id, Connection::Pointer conn) {
    auto w_player = world_server->GetPlayerManager().FindPlayer(account_id);
    if (!w_player) return;
    
    WorldSnapshotPacket snap;
    snap.map_id = w_player->map_id;
    snap.spawn_x = w_player->x;
    snap.spawn_y = w_player->y;
    snap.server_tick = current_server_tick;
    snap.server_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto visible = world_server->GetVisibilityManager().GetVisibleEntities(account_id);
    auto map = world_server->GetMapManager().GetMap(w_player->map_id);
    if (map) {
        auto map_players = map->GetPlayers();
        for (uint64_t eid : visible) {
            for (const auto& op : map_players) {
                if (op->entity_id == eid && op->account_id != account_id) {
                    PlayerData pd;
                    pd.account_id = op->account_id;
                    pd.character_id = op->character_id;
                    pd.map_id = op->map_id;
                    pd.x = op->x;
                    pd.y = op->y;
                    pd.direction = op->direction;
                    pd.movement_state = op->movement_state;
                    snap.players.push_back(pd);
                    break;
                }
            }
        }
    }
    Packet p;
    p.type = PacketType::kWorldSnapshot;
    p.payload = snap.Serialize();
    conn->SendPacket(p);
  };

  // Register Handlers
  dispatcher->RegisterHandler(PacketType::kAuthRequest, [=](Connection::Pointer conn, const Packet& p) {
    AuthRequestPacket req = AuthRequestPacket::Deserialize(p.payload);
    
    // Authenticate
    auto acc = account_manager->Login(req.username, req.password);
    AuthResponsePacket resp;
    
    if (acc) {
      std::string token = session_manager->CreateSession(*acc, conn);
      resp.success = true;
      resp.message = "Authenticated successfully";
      resp.token = token;
      resp.account_id = acc->id;
      
      
      // Add to player registry
      OnlinePlayer online_player;
      online_player.session_token = token;
      online_player.connection_id = conn->GetId();
      online_player.account_id = acc->id;
      online_player.character_id = resp.character_id;
      online_player.login_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      online_player.current_connection_state = PresenceState::kOnline;
      player_registry->AddPlayer(online_player);
      Logger::Info("User " + req.username + " logged in (Token: " + token.substr(0, 8) + "...)");
      
      // Broadcast Presence to others
      PlayerPresencePacket presence;
      presence.account_id = acc->id;
      presence.character_id = resp.character_id;
      presence.state = PresenceState::kOnline;
      
      Packet bcast_pkt;
      bcast_pkt.type = PacketType::kPlayerPresence;
      bcast_pkt.payload = presence.Serialize();
      
      for (const auto& other : player_registry->GetAllPlayers()) {
        if (other.account_id != acc->id) {
          auto other_session = session_manager->GetSessionByToken(other.session_token);
          if (other_session && other_session->connection) {
            other_session->connection->SendPacket(bcast_pkt);
          }
        }
      }
      
    } else {
      resp.success = false;
      resp.message = "Invalid credentials";
      Logger::Warn("Failed login attempt for user: " + req.username);
    }
    
    Packet out_p;
    out_p.type = PacketType::kAuthResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kCreateAccountRequest, [=](Connection::Pointer conn, const Packet& p) {
    AuthRequestPacket req = AuthRequestPacket::Deserialize(p.payload); // Using AuthRequest for username/password
    AuthResponsePacket resp;
    
    if (account_manager->CreateAccount(req.username, req.password)) {
        resp.success = true;
        resp.message = "Account created. Please log in.";
    } else {
        resp.success = false;
        resp.message = "Username already taken or invalid.";
    }
    
    Packet out_p;
    out_p.type = PacketType::kAuthResponse; // Client expects this or a specific create account response
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  
  dispatcher->RegisterHandler(PacketType::kCreateCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    CreateCharacterRequestPacket req = CreateCharacterRequestPacket::Deserialize(p.payload);
    CreateCharacterResponsePacket resp;
    
    auto character = character_repo->CreateCharacter(session->account.id, req.name, req.appearance);
    if (character) {
        resp.success = true;
        resp.message = "Character created successfully";
        resp.character_id = character->id;
    } else {
        resp.success = false;
        resp.message = "Failed to create character. Invalid name or name already taken.";
    }
    
    Packet out_p;
    out_p.type = PacketType::kCreateCharacterResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kDeleteCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    DeleteCharacterRequestPacket req = DeleteCharacterRequestPacket::Deserialize(p.payload);
    DeleteCharacterResponsePacket resp;
    
    if (character_repo->DeleteCharacter(session->account.id, req.character_id)) {
        resp.success = true;
        resp.message = "Character deleted.";
    } else {
        resp.success = false;
        resp.message = "Failed to delete character.";
    }
    
    Packet out_p;
    out_p.type = PacketType::kDeleteCharacterResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kCharacterListRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    CharacterListResponsePacket resp;
    auto chars = character_repo->LoadCharactersForAccount(session->account.id);
    for (const auto& c : chars) {
        CharacterEntry entry;
        entry.id = c.id;
        entry.name = c.name;
        entry.appearance = c.appearance;
        entry.map_id = c.map_id;
        entry.play_time_seconds = c.play_time_seconds;
        entry.money = c.money;
        entry.last_login = c.last_login;
        resp.characters.push_back(entry);
    }
    
    Packet out_p;
    out_p.type = PacketType::kCharacterListResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kSelectCharacterRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    SelectCharacterRequestPacket req = SelectCharacterRequestPacket::Deserialize(p.payload);
    SelectCharacterResponsePacket resp;
    
    auto character = character_repo->LoadCharacter(req.character_id);
    if (character && character->account_id == session->account.id) {
        resp.success = true;
        resp.message = "Character selected.";
        
        Packet out_p;
        out_p.type = PacketType::kSelectCharacterResponse;
        out_p.payload = resp.Serialize();
        conn->SendPacket(out_p);
        
        // Update online player state
        auto player = player_registry->GetPlayer(session->account.id);
        if (player) {
            player->character_id = character->id;
            
            // Load persistent blobs into memory
            auto persistence_state = persistence::PlayerPersistence::LoadPlayer(*db_pool, session->account.id);
            if (!persistence_state) {
                persistence_state = PlayerDataSyncPacket();
                persistence_state->account_id = session->account.id;
            }
            player->dirty_manager = std::make_shared<persistence::DirtyFlagManager>();
            player->persistence_state = std::make_shared<PlayerDataSyncPacket>(*persistence_state);
            
            player_registry->AddPlayer(*player);
            
            // Send PlayerDataSync
            Packet sync_p;
            sync_p.type = PacketType::kPlayerDataSync;
            sync_p.payload = persistence_state->Serialize();
            conn->SendPacket(sync_p);
            
            // Send CharacterLoadedResponse
            CharacterLoadedResponsePacket loaded_resp;
            loaded_resp.success = true;
            loaded_resp.message = "Character loaded and state synced.";
            Packet loaded_p;
            loaded_p.type = PacketType::kCharacterLoadedResponse;
            loaded_p.payload = loaded_resp.Serialize();
            conn->SendPacket(loaded_p);
            
            std::cout << "[SERVER] Selected character " << character->id << std::endl;
            inventory_mgr->LoadInventory(character->id);
            
            unboundmp::models::Inventory test_inv;
            if (inventory_mgr->GetInventory(character->id, test_inv)) {
                std::cout << "[SERVER] Loaded inventory for character. Item count: " << test_inv.items.size() << std::endl;
            } else {
                std::cout << "[SERVER] FAILED to load inventory for character!" << std::endl;
            }
            
            // --- Milestone 3: Register player into the world ---
            world_server->RegisterPlayer(
                session->account.id,
                character->id,
                p.session_token,
                character->name,
                player->persistence_state ? player->persistence_state->map_id : 0,
                player->persistence_state ? player->persistence_state->x : 0.0f,
                player->persistence_state ? player->persistence_state->y : 0.0f);
            
            SendWorldSnapshot(session->account.id, conn);
            
            Logger::Info("Account " + std::to_string(session->account.id) + " selected character " + std::to_string(character->id));
        }
    } else {
        resp.success = false;
        resp.message = "Character not found or ownership mismatch.";
        Logger::Warn("Failed character selection for account " + std::to_string(session->account.id) + " character " + std::to_string(req.character_id));
        
        Packet out_p;
        out_p.type = PacketType::kSelectCharacterResponse;
        out_p.payload = resp.Serialize();
        conn->SendPacket(out_p);
    }
  });

  dispatcher->RegisterHandler(PacketType::kPing, [=](Connection::Pointer conn, const Packet& p) {
      Packet out_p;
      out_p.type = PacketType::kPong;
      out_p.payload = p.payload;
      out_p.session_token = p.session_token;
      conn->SendPacket(out_p);
  });

  dispatcher->RegisterHandler(PacketType::kHeartbeat, [=](Connection::Pointer conn, const Packet& p) {
    if (p.session_token.empty()) return;
    
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    session_manager->UpdateActivity(p.session_token);
    
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (session) {
      player_registry->UpdateHeartbeat(session->account.id, now);
    }
  });

  dispatcher->RegisterHandler(PacketType::kPokemonUpdate, [=](Connection::Pointer conn, const Packet& p) {
    PokemonUpdatePacket update = PokemonUpdatePacket::Deserialize(p.payload);
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (session) {
      auto player = player_registry->GetPlayer(session->account.id);
      if (player && player->persistence_state && player->dirty_manager) {
          player->persistence_state->party_slots = update.party_slots;
          player->persistence_state->pc_blob = update.pc_blob;
          player->dirty_manager->MarkDirty(persistence::DirtyComponent::kParty | persistence::DirtyComponent::kPc);
      }
    }
  });

  dispatcher->RegisterHandler(PacketType::kStoryUpdate, [=](Connection::Pointer conn, const Packet& p) {
    StoryUpdatePacket update = StoryUpdatePacket::Deserialize(p.payload);
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (session) {
      auto player = player_registry->GetPlayer(session->account.id);
      if (player && player->persistence_state && player->dirty_manager) {
          player->persistence_state->story_flags = update.story_flags;
          player->persistence_state->story_badges = update.story_badges;
          player->persistence_state->story_quests = update.story_quests;
          player->dirty_manager->MarkDirty(persistence::DirtyComponent::kStory);
      }
    }
  });

  dispatcher->RegisterHandler(PacketType::kMapTransition, [=, &current_server_tick](Connection::Pointer conn, const Packet& p) {
    MapTransitionPacket update = MapTransitionPacket::Deserialize(p.payload);
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (session) {
      auto player = player_registry->GetPlayer(session->account.id);
      if (player && player->persistence_state && player->dirty_manager) {
          player->persistence_state->map_id = update.map_id;
          player->persistence_state->x = update.x;
          player->persistence_state->y = update.y;
          player->dirty_manager->MarkDirty(persistence::DirtyComponent::kWorldState);
          
          world_server->TransferPlayer(session->account.id, update.map_id, update.x, update.y);
          SendWorldSnapshot(session->account.id, conn);
      }
    }
  });

  dispatcher->RegisterHandler(PacketType::kMoveRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    if (!session) return;
    
    MoveRequestPacket req = MoveRequestPacket::Deserialize(p.payload);
    
    if (world_server->ProcessMovementRequest(session->account.id, req.x, req.y, req.direction, req.movement_state)) {
        // Acknowledge locally
        MoveAcceptedPacket ack;
        ack.x = req.x;
        ack.y = req.y;
        Packet ack_p;
        ack_p.type = PacketType::kMoveAccepted;
        ack_p.payload = ack.Serialize();
        conn->SendPacket(ack_p);
        
        // Broadcast movement to all players who can see this player
        PlayerMovePacket move;
        move.account_id = session->account.id;
        move.x = req.x;
        move.y = req.y;
        move.movement_state = req.movement_state;
        
        Packet move_p;
        move_p.type = PacketType::kPlayerMove;
        move_p.payload = move.Serialize();
        
        PlayerDirectionPacket dir;
        dir.account_id = session->account.id;
        dir.direction = req.direction;
        
        Packet dir_p;
        dir_p.type = PacketType::kPlayerDirection;
        dir_p.payload = dir.Serialize();
        
        for (const auto& other_player : player_registry->GetAllPlayers()) {
            if (other_player.account_id != session->account.id) {
                if (world_server->GetVisibilityManager().CanSee(other_player.account_id, session->account.id)) {
                    auto other_session = session_manager->GetSessionByToken(other_player.session_token);
                    if (other_session && other_session->connection) {
                        other_session->connection->SendPacket(move_p);
                        other_session->connection->SendPacket(dir_p);
                    }
                }
            }
        }
    } else {
        MoveRejectedPacket rej;
        rej.x = req.x;
        rej.y = req.y;
        Packet rej_p;
        rej_p.type = PacketType::kMoveRejected;
        rej_p.payload = rej.Serialize();
        conn->SendPacket(rej_p);
        conn->SendPacket(rej_p);
    }
  });

  dispatcher->RegisterHandler(PacketType::kInventoryRequest, [=](Connection::Pointer conn, const Packet& p) {
      std::cout << "[SERVER] Received kInventoryRequest" << std::endl;
      auto session = session_manager->GetSessionByToken(p.session_token);
      if (!session) {
          std::cout << "[SERVER] kInventoryRequest Failed: No session" << std::endl;
          return;
      }
      auto player = player_registry->GetPlayer(session->account.id);
      if (!player) {
          std::cout << "[SERVER] kInventoryRequest Failed: No player" << std::endl;
          return;
      }
      
      unboundmp::models::Inventory inv;
      if (inventory_mgr->GetInventory(player->character_id, inv)) {
          std::cout << "[SERVER] Sending kInventoryResponse with " << inv.items.size() << " items" << std::endl;
          InventoryResponsePacket resp;
          resp.inventory = inv;
          Packet out_p;
          out_p.type = PacketType::kInventoryResponse;
          out_p.payload = resp.Serialize();
          conn->SendPacket(out_p);
      }
  });

  dispatcher->RegisterHandler(PacketType::kItemMoved, [=](Connection::Pointer conn, const Packet& p) {
      auto session = session_manager->GetSessionByToken(p.session_token);
      if (!session) return;
      auto player = player_registry->GetPlayer(session->account.id);
      if (!player) return;
      
      ItemMovedPacket req = ItemMovedPacket::Deserialize(p.payload);
      if (!inventory_mgr->MoveItem(player->character_id, req.from_slot, req.to_slot, req.amount)) {
          InventoryErrorPacket err;
          err.message = "Invalid item move.";
          Packet err_p;
          err_p.type = PacketType::kInventoryError;
          err_p.payload = err.Serialize();
          conn->SendPacket(err_p);
      } else {
          // Send update (could be optimized to delta)
          unboundmp::models::Inventory inv;
          if (inventory_mgr->GetInventory(player->character_id, inv)) {
              InventoryUpdatePacket up;
              up.version_number = inv.version_number;
              // For simplicity in this milestone, send full inventory as update or just changed items.
              // A true delta requires tracking changes per request, so here we just pack everything in `changed_items` that has qty>0
              for (const auto& item : inv.items) {
                  if (item.quantity > 0) up.changed_items.push_back(item);
              }
              Packet up_p;
              up_p.type = PacketType::kInventoryUpdate;
              up_p.payload = up.Serialize();
              conn->SendPacket(up_p);
          }
      }
  });

  dispatcher->RegisterHandler(PacketType::kItemUsed, [=](Connection::Pointer conn, const Packet& p) {
      auto session = session_manager->GetSessionByToken(p.session_token);
      if (!session) return;
      auto player = player_registry->GetPlayer(session->account.id);
      if (!player) return;
      
      ItemUsedPacket req = ItemUsedPacket::Deserialize(p.payload);
      if (!item_mgr->ConsumeItem(player->character_id, req.slot, req.target_entity)) {
          InventoryErrorPacket err;
          err.message = "Cannot use that item.";
          Packet err_p;
          err_p.type = PacketType::kInventoryError;
          err_p.payload = err.Serialize();
          conn->SendPacket(err_p);
      } else {
          unboundmp::models::Inventory inv;
          if (inventory_mgr->GetInventory(player->character_id, inv)) {
              InventoryUpdatePacket up;
              up.version_number = inv.version_number;
              for (const auto& item : inv.items) {
                  if (item.quantity > 0) up.changed_items.push_back(item);
              }
              Packet up_p;
              up_p.type = PacketType::kInventoryUpdate;
              up_p.payload = up.Serialize();
              conn->SendPacket(up_p);
          }
      }
  });

  dispatcher->RegisterHandler(PacketType::kItemDropped, [=](Connection::Pointer conn, const Packet& p) {
      auto session = session_manager->GetSessionByToken(p.session_token);
      if (!session) return;
      auto player = player_registry->GetPlayer(session->account.id);
      if (!player) return;
      
      ItemDroppedPacket req = ItemDroppedPacket::Deserialize(p.payload);
      if (!inventory_mgr->RemoveItem(player->character_id, req.slot, req.amount)) {
          InventoryErrorPacket err;
          err.message = "Failed to drop item.";
          Packet err_p;
          err_p.type = PacketType::kInventoryError;
          err_p.payload = err.Serialize();
          conn->SendPacket(err_p);
      } else {
          unboundmp::models::Inventory inv;
          if (inventory_mgr->GetInventory(player->character_id, inv)) {
              InventoryUpdatePacket up;
              up.version_number = inv.version_number;
              for (const auto& item : inv.items) {
                  if (item.quantity > 0) up.changed_items.push_back(item);
              }
              // Ideally removed slots list is tracked, but sending full state works for milestone.
              Packet up_p;
              up_p.type = PacketType::kInventoryUpdate;
              up_p.payload = up.Serialize();
              conn->SendPacket(up_p);
          }
      }
  });

  dispatcher->RegisterHandler(PacketType::kLogoutRequest, [=](Connection::Pointer conn, const Packet& p) {
    auto session = session_manager->GetSessionByToken(p.session_token);
    LogoutResponsePacket resp;
    resp.success = false;
    
    if (session) {
      auto player = player_registry->GetPlayer(session->account.id);
      if (player) {
          // Flush persistence immediately
          if (player->dirty_manager && player->dirty_manager->AnyDirty()) {
              uint32_t flags = player->dirty_manager->GetFlagsAndClear();
              persistence::PlayerPersistence::SavePlayer(*db_pool, player->account_id, flags, *player->persistence_state);
          }
          // Remove from world
          world_server->RemovePlayer(session->account.id);
          
          inventory_mgr->UnloadInventory(player->character_id);
          
          // Remove from registry
          player_registry->RemovePlayer(session->account.id);
      }
      // End session
      session_manager->DisconnectSession(p.session_token);
      resp.success = true;
      Logger::Info("Account " + std::to_string(session->account.id) + " logged out normally.");
    }
    
    Packet out_p;
    out_p.type = PacketType::kLogoutResponse;
    out_p.payload = resp.Serialize();
    conn->SendPacket(out_p);
  });

  // Start Networking
  auto network_manager = ctx.GetNetworkManager();
  network_manager->StartServer(4000);
  Logger::Info("Server running on port 4000");

  auto* server = network_manager->GetServer();
  
  bool running = true;
  auto last_tick = std::chrono::steady_clock::now();
  
  while (running) {
    server->PollIncoming(dispatcher.get());
    
    // --- Milestone 3/4: World tick & Visibility Sync ---
    auto visibility_events = world_server->Update();
    for (const auto& ev : visibility_events) {
        if (!ev.entity) continue;
        
        auto observer_player = player_registry->GetPlayer(ev.observer_account_id);
        if (!observer_player) continue;
        
        auto session = session_manager->GetSessionByToken(observer_player->session_token);
        if (!session || !session->connection) continue;
        
        if (ev.is_spawn) {
            // Cast to PlayerEntity to get the actual account_id
            auto player_entity = std::dynamic_pointer_cast<world::PlayerEntity>(ev.entity);
            if (!player_entity) continue;
            
            PlayerSpawnPacket spawn;
            spawn.account_id = player_entity->account_id;
            spawn.character_id = player_entity->character_id;
            spawn.map_id = player_entity->map_id;
            spawn.x = player_entity->x;
            spawn.y = player_entity->y;
            spawn.direction = player_entity->direction;
            spawn.movement_state = player_entity->movement_state;
            
            Packet p;
            p.type = PacketType::kPlayerSpawn;
            p.payload = spawn.Serialize();
            session->connection->SendPacket(p);
        } else {
            auto despawn_player = std::dynamic_pointer_cast<world::PlayerEntity>(ev.entity);
            PlayerDespawnPacket despawn;
            despawn.account_id = despawn_player ? despawn_player->account_id : ev.entity->entity_id;
            
            Packet p;
            p.type = PacketType::kPlayerDespawn;
            p.payload = despawn.Serialize();
            session->connection->SendPacket(p);
        }
    }
    
    session_manager->KickInactiveSessions(3600);
    
    // Strict 50ms tick pacing (20 TPS)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count();
    if (elapsed < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    last_tick = std::chrono::steady_clock::now();
    current_server_tick++;
  }
  
  ctx.Shutdown();
  Logger::Info("Server shut down.");
  
  return 0;
}
